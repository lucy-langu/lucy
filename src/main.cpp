#include "lucy/lucy.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

constexpr const char* LUCY_VERSION = "2.0.7";

void print_usage() {
    std::cout
        << "Lucy " << LUCY_VERSION << "\n"
        << "Usage:\n"
        << "  lucy <file.lucy> [arguments...]\n"
        << "  lucy run <command> [arguments...]\n"
        << "  lucy commands\n"
        << "  lucy install-binstubs [directory]\n"
        << "  lucy -i | --interactive\n\n"
        << "Options:\n"
        << "  -i, --interactive  Start the REPL\n"
        << "  -h, --help         Show this help\n"
        << "  -v, --version      Show version\n\n"
        << "Project commands are declared in lucy.toml under [commands].\n"
        << "Script arguments are available in ARGV (and argv).\n";
}

std::string trim(std::string value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return {};
    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

std::string unquote_toml_string(const std::string& value) {
    const std::string v = trim(value);
    if (v.size() < 2 || v.front() != '"' || v.back() != '"')
        throw std::runtime_error("lucy.toml: command entries must be quoted strings");
    std::string out;
    for (std::size_t i = 1; i + 1 < v.size(); ++i) {
        if (v[i] == '\\') {
            if (++i + 1 > v.size()) throw std::runtime_error("lucy.toml: invalid escape");
            if (v[i] == '"' || v[i] == '\\') out += v[i];
            else throw std::runtime_error("lucy.toml: unsupported escape in command path");
        } else {
            out += v[i];
        }
    }
    return out;
}

std::map<std::string, std::string> load_commands(const fs::path& manifest) {
    std::ifstream input(manifest);
    if (!input) throw std::runtime_error("cannot read " + manifest.string());
    std::map<std::string, std::string> commands;
    std::string line;
    bool in_commands = false;
    std::size_t line_number = 0;
    while (std::getline(input, line)) {
        ++line_number;
        bool quoted = false, escaped = false;
        std::size_t comment = std::string::npos;
        for (std::size_t i = 0; i < line.size(); ++i) {
            if (escaped) { escaped = false; continue; }
            if (quoted && line[i] == '\\') { escaped = true; continue; }
            if (line[i] == '"') quoted = !quoted;
            else if (line[i] == '#' && !quoted) { comment = i; break; }
        }
        if (comment != std::string::npos) line.erase(comment);
        line = trim(line);
        if (line.empty()) continue;
        if (line.front() == '[' && line.back() == ']') {
            in_commands = line == "[commands]";
            continue;
        }
        if (!in_commands) continue;
        const auto equal = line.find('=');
        if (equal == std::string::npos)
            throw std::runtime_error(manifest.string() + ":" + std::to_string(line_number) + ": expected key = \"path\"");
        std::string name = trim(line.substr(0, equal));
        if (name.empty() || !std::isalpha(static_cast<unsigned char>(name[0])))
            throw std::runtime_error(manifest.string() + ":" + std::to_string(line_number) + ": invalid command name");
        for (char c : name) {
            if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '_'))
                throw std::runtime_error(manifest.string() + ":" + std::to_string(line_number) + ": invalid command name");
        }
        const std::string path = unquote_toml_string(line.substr(equal + 1));
        if (path.empty()) throw std::runtime_error(manifest.string() + ":" + std::to_string(line_number) + ": command path cannot be empty");
        if (!commands.emplace(name, path).second)
            throw std::runtime_error(manifest.string() + ":" + std::to_string(line_number) + ": duplicate command '" + name + "'");
    }
    return commands;
}

fs::path find_project(fs::path start) {
    std::error_code ec;
    start = fs::absolute(start, ec);
    if (ec) start = fs::current_path();
    if (!fs::is_directory(start)) start = start.parent_path();
    while (!start.empty()) {
        if (fs::exists(start / "lucy.toml")) return start;
        const fs::path parent = start.parent_path();
        if (parent == start) break;
        start = parent;
    }
    throw std::runtime_error("no lucy.toml found in this directory or its parents");
}

int execute_project_command(const fs::path& project, const std::string& name,
                            const std::vector<std::string>& args) {
    const auto commands = load_commands(project / "lucy.toml");
    const auto found = commands.find(name);
    if (found == commands.end()) {
        std::string available;
        for (const auto& item : commands) {
            if (!available.empty()) available += ", ";
            available += item.first;
        }
        throw std::runtime_error("unknown project command '" + name + "'" +
            (available.empty() ? "" : "; available: " + available));
    }
    const fs::path relative_entry(found->second);
    if (relative_entry.is_absolute())
        throw std::runtime_error("command '" + name + "' must use a project-relative path");
    const fs::path project_root = fs::weakly_canonical(project);
    const fs::path script = fs::weakly_canonical(project_root / relative_entry);
    const fs::path relative_script = fs::relative(script, project_root);
    if (relative_script.empty() || *relative_script.begin() == "..")
        throw std::runtime_error("command '" + name + "' resolves outside the project directory");
    if (!fs::exists(script) || !fs::is_regular_file(script))
        throw std::runtime_error("command '" + name + "' points to a missing file: " + script.string());
    // Keep the caller's directory available to scaffolding commands, while
    // running project-bound tools from a deterministic project-root cwd.
    const std::string caller_directory = fs::current_path().string();
#ifdef _WIN32
    _putenv_s("LUCY_INVOKED_FROM", caller_directory.c_str());
#else
    setenv("LUCY_INVOKED_FROM", caller_directory.c_str(), 1);
#endif
    fs::current_path(project_root);
    lucy::run_file(script.string(), args);
    return 0;
}

fs::path default_bin_directory() {
#ifdef _WIN32
    const char* local = std::getenv("LOCALAPPDATA");
    if (local && *local) return fs::path(local) / "Lucy" / "bin";
    const char* profile = std::getenv("USERPROFILE");
    if (profile && *profile) return fs::path(profile) / "AppData" / "Local" / "Lucy" / "bin";
    return fs::current_path() / "Lucy-bin";
#else
    const char* home = std::getenv("HOME");
    if (home && *home) return fs::path(home) / ".local" / "bin";
    return fs::current_path() / ".lucy" / "bin";
#endif
}

std::string shell_quote(const std::string& value) {
    std::string out = "'";
    for (char c : value) {
        if (c == '\'') out += "'\\''";
        else out += c;
    }
    return out + "'";
}

int install_binstubs(const fs::path& project, fs::path bin_dir, const std::string& executable) {
    const auto commands = load_commands(project / "lucy.toml");
    if (commands.empty()) {
        std::cout << "No [commands] entries found in " << (project / "lucy.toml").string() << "\n";
        return 0;
    }
    fs::create_directories(bin_dir);
    std::size_t installed = 0;
    for (const auto& item : commands) {
#ifdef _WIN32
        const fs::path stub = bin_dir / (item.first + ".cmd");
        std::ofstream out(stub, std::ios::binary);
        if (!out) throw std::runtime_error("cannot write binstub: " + stub.string());
        out << "@echo off\r\n"
            << "\"" << executable << "\" --project-command \"" << project.string()
            << "\" \"" << item.first << "\" %*\r\n";
#else
        const fs::path stub = bin_dir / item.first;
        std::ofstream out(stub);
        if (!out) throw std::runtime_error("cannot write binstub: " + stub.string());
        out << "#!/bin/sh\nexec " << shell_quote(executable)
            << " --project-command " << shell_quote(project.string())
            << " " << shell_quote(item.first) << " \"$@\"\n";
        out.close();
        fs::permissions(stub, fs::perms::owner_exec | fs::perms::owner_read | fs::perms::owner_write |
                              fs::perms::group_exec | fs::perms::group_read |
                              fs::perms::others_exec | fs::perms::others_read,
                        fs::perm_options::replace);
#endif
        ++installed;
        std::cout << "  " << item.first << " -> " << stub.string() << "\n";
    }
    std::cout << "Installed " << installed << " Lucy binstub(s) in " << bin_dir.string() << "\n";
#ifndef _WIN32
    std::cout << "Ensure this directory is on PATH (for example: export PATH=\"$HOME/.local/bin:$PATH\").\n";
#endif
    return 0;
}


#ifdef LUCY_PAW_BUILD
void set_environment_value(const char* key, const std::string& value) {
#ifdef _WIN32
    _putenv_s(key, value.c_str());
#else
    setenv(key, value.c_str(), 1);
#endif
}

fs::path first_existing(const std::vector<fs::path>& candidates, const std::string& description) {
    for (const auto& candidate : candidates) {
        std::error_code ec;
        if (fs::exists(candidate, ec) && !ec) return fs::weakly_canonical(candidate);
    }
    throw std::runtime_error("cannot locate " + description + "; reinstall Lucy/Paw or set PAW_HOME");
}

int run_paw(int argc, char** argv) {
    std::error_code ec;
    fs::path executable = fs::absolute(argv[0], ec);
    if (ec) executable = fs::path(argv[0]);
    const fs::path exe_dir = executable.parent_path();

    fs::path runtime = exe_dir / "lucy";
#ifdef _WIN32
    runtime += ".exe";
#endif
    if (!fs::exists(runtime)) {
        const char* from_env = std::getenv("LUCY_RUNTIME");
        if (from_env && *from_env) runtime = fs::path(from_env);
    }
    if (!fs::exists(runtime)) {
        throw std::runtime_error("Paw cannot find the Lucy runtime beside itself; install lucy and paw together");
    }

    std::vector<fs::path> paw_candidates;
    const char* configured_home = std::getenv("PAW_HOME");
    if (configured_home && *configured_home) paw_candidates.emplace_back(fs::path(configured_home) / "main.lucy");
#ifdef _WIN32
    paw_candidates.emplace_back(exe_dir / "tools" / "paw" / "main.lucy");
#else
    paw_candidates.emplace_back(exe_dir / ".." / "share" / "lucy" / "tools" / "paw" / "main.lucy");
    paw_candidates.emplace_back(exe_dir / "tools" / "paw" / "main.lucy");
#endif
#ifdef LUCY_SOURCE_DIR
    paw_candidates.emplace_back(fs::path(LUCY_SOURCE_DIR) / "tools" / "paw" / "main.lucy");
#endif
    const fs::path paw_main = first_existing(paw_candidates, "Paw's main.lucy");

    std::vector<fs::path> stdlib_candidates;
#ifdef _WIN32
    stdlib_candidates.emplace_back(exe_dir / "stdlib");
#else
    stdlib_candidates.emplace_back(exe_dir / ".." / "share" / "lucy" / "stdlib");
    stdlib_candidates.emplace_back(exe_dir / "stdlib");
#endif
#ifdef LUCY_SOURCE_DIR
    stdlib_candidates.emplace_back(fs::path(LUCY_SOURCE_DIR) / "stdlib");
#endif
    const fs::path stdlib = first_existing(stdlib_candidates, "Lucy standard library");

    fs::path installation_root;
#ifdef _WIN32
    installation_root = exe_dir;
#else
    installation_root = exe_dir.parent_path();
#endif
    std::error_code root_ec;
    if (!fs::exists(installation_root / "stdlib", root_ec) &&
        !fs::exists(installation_root / "share" / "lucy" / "stdlib", root_ec)) {
#ifdef LUCY_SOURCE_DIR
        installation_root = fs::path(LUCY_SOURCE_DIR);
#else
        installation_root = stdlib.parent_path();
#endif
    }

    set_environment_value("PAW_HOME", paw_main.parent_path().string());
    set_environment_value("PAW_RUNTIME", fs::weakly_canonical(runtime).string());
    // LUCY_PATH is the installation/project root, not the stdlib subdirectory.
    // Preserve an explicit user override; otherwise point it at the detected root.
    if (!std::getenv("LUCY_PATH") || !*std::getenv("LUCY_PATH"))
        set_environment_value("LUCY_PATH", fs::weakly_canonical(installation_root).string());

    std::vector<std::string> args;
    for (int i = 1; i < argc; ++i) args.emplace_back(argv[i]);
    lucy::run_file(paw_main.string(), args);
    return 0;
}
#endif

} // namespace

int main(int argc, char** argv) {
    try {
#ifdef LUCY_PAW_BUILD
        return run_paw(argc, argv);
#endif
        if (argc == 1) {
            lucy::repl();
            return 0;
        }

        const std::string command = argv[1];
        if (command == "-i" || command == "--interactive") {
            lucy::repl();
            return 0;
        }
        if (command == "-h" || command == "--help") {
            print_usage();
            return 0;
        }
        if (command == "-v" || command == "--version") {
            std::cout << LUCY_VERSION << "\n";
            return 0;
        }

        if (command == "--project-command") {
            if (argc < 4) throw std::runtime_error("usage: lucy --project-command <project-dir> <name> [arguments...]");
            std::vector<std::string> args;
            for (int i = 4; i < argc; ++i) args.emplace_back(argv[i]);
            return execute_project_command(argv[2], argv[3], args);
        }

        if (command == "commands" || command == "run" || command == "install-binstubs") {
            const fs::path project = find_project(fs::current_path());
            const auto commands = load_commands(project / "lucy.toml");
            if (command == "commands") {
                if (commands.empty()) std::cout << "No project commands defined.\n";
                for (const auto& item : commands)
                    std::cout << item.first << "\t" << item.second << "\n";
                return 0;
            }
            if (command == "run") {
                if (argc < 3) throw std::runtime_error("usage: lucy run <command> [arguments...]");
                std::vector<std::string> args;
                for (int i = 3; i < argc; ++i) args.emplace_back(argv[i]);
                return execute_project_command(project, argv[2], args);
            }
            const fs::path bin_dir = argc > 2 ? fs::path(argv[2]) : default_bin_directory();
            std::error_code ec;
            fs::path exe = fs::absolute(argv[0], ec);
            if (ec || !fs::exists(exe)) exe = "lucy";
            return install_binstubs(project, bin_dir, exe.string());
        }

        std::vector<std::string> script_arguments;
        for (int index = 2; index < argc; ++index) script_arguments.emplace_back(argv[index]);
        lucy::run_file(command, script_arguments);
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "lucy: " << error.what() << '\n';
        return 1;
    }
}

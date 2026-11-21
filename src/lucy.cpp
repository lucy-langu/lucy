#include "lucy/lucy.hpp"
#include "lucy/lexer.hpp"
#include "lucy/parser.hpp"
#include "lucy/runtime.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <regex>
#include <vector>

using namespace lucy;

void lucy::run_source(
    const std::string& source,
    const std::string& name,
    const std::vector<std::string>& argv) {
    try {
        Lexer lexer(source);
        Parser parser(lexer.scan());
        Interpreter interpreter(argv);
        interpreter.set_current_file(name);
        interpreter.run(parser.parse());
    } catch (const std::exception& error) {
        const std::string message = error.what();
        std::smatch match;
        static const std::regex parser_location(R"(line ([0-9]+), column ([0-9]+))");
        static const std::regex lexer_location(R"( at ([0-9]+):([0-9]+))");
        static const std::regex runtime_location(R"(\[Lucy source: .*:([0-9]+):([0-9]+)\])");
        std::size_t line = 0, column = 0;
        if (std::regex_search(message, match, parser_location) ||
            std::regex_search(message, match, lexer_location) ||
            std::regex_search(message, match, runtime_location)) {
            try {
                line = static_cast<std::size_t>(std::stoul(match[1].str()));
                column = static_cast<std::size_t>(std::stoul(match[2].str()));
            } catch (...) { line = column = 0; }
        }

        std::ostringstream diagnostic;
        diagnostic << "in " << name << ": " << message;
        // Lexical and parser diagnostics have reliable token coordinates. Show the
        // actual source line and caret rather than inventing runtime locations.
        if (line > 0 && column > 0) {
            std::istringstream lines(source);
            std::string source_line;
            bool found_line = false;
            for (std::size_t index = 1; index <= line && std::getline(lines, source_line); ++index) {
                if (index == line) { found_line = true; break; }
            }
            // EOF after a trailing newline is a valid source position even though
            // getline has no physical text row to return for that final empty line.
            if (!found_line) source_line.clear();
            diagnostic << "\n --> " << name << ':' << line << ':' << column
                       << "\n  |\n" << line << " | " << source_line
                       << "\n  | " << std::string(column - 1, ' ') << '^';
        }
        throw std::runtime_error(diagnostic.str());
    }
}

void lucy::run_file(
    const std::string& path,
    const std::vector<std::string>& argv) {
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error(
            "IOError: cannot open source file '" + path + "'");
    }

    std::ostringstream source;
    source << file.rdbuf();
    run_source(source.str(), path, argv);
}

void lucy::repl() {
    Interpreter interpreter;
    interpreter.repl();
}

#include "lucy/lucy.hpp"

#include <iostream>
#include <string>
#include <vector>

namespace {

void print_usage() {
    std::cout
        << "Lucy 2.0.0\n"
        << "Usage:\n"
        << "  lucy <file.lucy> [arguments...]\n"
        << "  lucy -i\n\n"
        << "Options:\n"
        << "  -i, --interactive  Start the REPL\n"
        << "  -h, --help         Show this help\n"
        << "  -v, --version      Show version\n"
        << "\n"
        << "Script arguments are available in ARGV (and argv).\n";
}

} // namespace

int main(int argc, char** argv) {
    try {
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
            std::cout << "2.0.0\n";
            return 0;
        }

        std::vector<std::string> script_arguments;
        for (int index = 2; index < argc; ++index) {
            script_arguments.emplace_back(argv[index]);
        }

        lucy::run_file(command, script_arguments);
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "lucy: " << error.what() << '\n';
        return 1;
    }
}

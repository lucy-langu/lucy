#include "lucy/repl.hpp"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

namespace lucy {
namespace {

constexpr std::size_t kMaxHistory = 1000;

std::string history_file_path() {
#ifdef _WIN32
    const char* home = std::getenv("USERPROFILE");
    if (home) {
        return std::string(home) + "\\.lucy_history";
    }
    return ".lucy_history";
#else
    const char* home = std::getenv("HOME");
    if (home) {
        return std::string(home) + "/.lucy_history";
    }
    return ".lucy_history";
#endif
}

#ifdef _WIN32

void enable_virtual_terminal() {
    HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
    if (output == INVALID_HANDLE_VALUE) {
        return;
    }

    DWORD mode = 0;
    if (!GetConsoleMode(output, &mode)) {
        return;
    }

    SetConsoleMode(output, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
}

int read_key() {
    const int first = _getch();
    if (first == 0 || first == 224) {
        return 0x100 + _getch();
    }
    return first;
}

#else

class RawTerminal {
public:
    RawTerminal() : active_(false), original_{} {}

    bool enable() {
        if (!isatty(STDIN_FILENO)) {
            return false;
        }

        if (tcgetattr(STDIN_FILENO, &original_) != 0) {
            return false;
        }

        termios raw = original_;
        raw.c_lflag &= static_cast<unsigned>(~(ICANON | ECHO));
        raw.c_iflag &= static_cast<unsigned>(~(IXON | ICRNL));
        raw.c_oflag |= OPOST;
        raw.c_cc[VMIN] = 1;
        raw.c_cc[VTIME] = 0;

        if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) != 0) {
            return false;
        }

        active_ = true;
        return true;
    }

    ~RawTerminal() {
        if (active_) {
            tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_);
        }
    }

private:
    bool active_;
    termios original_;
};

int read_key() {
    unsigned char ch = 0;
    if (read(STDIN_FILENO, &ch, 1) != 1) {
        return -1;
    }

    if (ch != 0x1b) {
        return ch;
    }

    unsigned char sequence[2] = {0, 0};
    if (read(STDIN_FILENO, &sequence[0], 1) != 1) {
        return 0x1b;
    }

    if (sequence[0] != '[' && sequence[0] != 'O') {
        return 0x1b;
    }

    if (read(STDIN_FILENO, &sequence[1], 1) != 1) {
        return 0x1b;
    }

    switch (sequence[1]) {
    case 'A': return 0x101; // Up
    case 'B': return 0x102; // Down
    case 'C': return 0x103; // Right
    case 'D': return 0x104; // Left
    case 'H': return 0x105; // Home
    case 'F': return 0x106; // End
    case '3': {
        unsigned char tilde = 0;
        if (read(STDIN_FILENO, &tilde, 1) == 1 && tilde == '~') {
            return 0x107; // Delete
        }
        return 0x1b;
    }
    default:
        return 0x1b;
    }
}

#endif

void render_line(const std::string& prompt, const std::string& line,
                 std::size_t cursor) {
    std::cout << '\r' << prompt << line << "\x1b[K";
    const std::size_t distance = line.size() - cursor;
    if (distance > 0) {
        std::cout << "\x1b[" << distance << 'D';
    }
    std::cout.flush();
}

std::string longest_common_prefix(const std::vector<std::string>& values) {
    if (values.empty()) {
        return {};
    }

    std::string result = values.front();
    for (std::size_t i = 1; i < values.size(); ++i) {
        std::size_t length = 0;
        while (length < result.size() && length < values[i].size() &&
               result[length] == values[i][length]) {
            ++length;
        }
        result.resize(length);
    }
    return result;
}

} // namespace

ReplLineEditor::ReplLineEditor()
    : history_path_(history_file_path()) {
#ifdef _WIN32
    enable_virtual_terminal();
#endif
    load_history();
}

ReplLineEditor::~ReplLineEditor() = default;

void ReplLineEditor::print_history() const {
    const std::size_t first = history_.size() > 100 ? history_.size() - 100 : 0;
    for (std::size_t i = first; i < history_.size(); ++i) {
        std::cout << (i + 1) << "  " << history_[i] << '\n';
    }
}

void ReplLineEditor::load_history() {
    std::ifstream input(history_path_);
    if (!input) {
        return;
    }

    std::string line;
    while (std::getline(input, line)) {
        if (!line.empty()) {
            history_.push_back(line);
        }
    }

    if (history_.size() > kMaxHistory) {
        history_.erase(history_.begin(),
                       history_.end() - static_cast<std::ptrdiff_t>(kMaxHistory));
    }
}

void ReplLineEditor::save_history_entry(const std::string& line) {
    std::ofstream output(history_path_, std::ios::app);
    if (output) {
        output << line << '\n';
    }
}

void ReplLineEditor::add_history(const std::string& line) {
    if (line.empty()) {
        return;
    }

    if (!history_.empty() && history_.back() == line) {
        history_index_ = history_.size();
        return;
    }

    auto duplicate = std::find(history_.begin(), history_.end(), line);
    if (duplicate != history_.end()) {
        history_.erase(duplicate);
    }

    history_.push_back(line);
    if (history_.size() > kMaxHistory) {
        history_.erase(history_.begin());
    }

    save_history_entry(line);
    history_index_ = history_.size();
}

bool ReplLineEditor::read_line(const std::string& prompt, std::string& line,
                               const CompletionProvider& completer) {
#ifdef _WIN32
    DWORD input_mode = 0;
    if (!GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &input_mode)) {
        if (!std::getline(std::cin, line)) {
            return false;
        }
        add_history(line);
        return true;
    }
#else
    if (!isatty(STDIN_FILENO)) {
        if (!std::getline(std::cin, line)) {
            return false;
        }
        add_history(line);
        return true;
    }
#endif

#ifndef _WIN32
    RawTerminal terminal;
    if (!terminal.enable()) {
        if (!std::getline(std::cin, line)) {
            return false;
        }
        add_history(line);
        return true;
    }
#endif

    line.clear();
    std::size_t cursor = 0;
    history_index_ = history_.size();
    bool completion_listed = false;
    std::string last_completion_prefix;
    std::vector<std::string> completion_matches;
    std::size_t completion_index = 0;
    std::size_t completion_token_start = 0;

    std::cout << prompt << std::flush;

    while (true) {
        const int key = read_key();
        if (key < 0) {
            std::cout << '\n';
            return false;
        }

        if (key == '\r' || key == '\n') {
            std::cout << '\n';
            add_history(line);
            return true;
        }

        if (key == 0x04) { // Ctrl-D
            if (line.empty()) {
                std::cout << '\n';
                return false;
            }
            continue;
        }

        if (key == 0x7f || key == '\b') {
            if (cursor > 0) {
                line.erase(cursor - 1, 1);
                --cursor;
                render_line(prompt, line, cursor);
            }
            completion_listed = false;
            continue;
        }

        if (key == 0x101 || key == (0x100 + 72)) { // Up
            if (!history_.empty() && history_index_ > 0) {
                --history_index_;
                line = history_[history_index_];
                cursor = line.size();
                render_line(prompt, line, cursor);
            }
            completion_listed = false;
            continue;
        }

        if (key == 0x102 || key == (0x100 + 80)) { // Down
            if (history_index_ + 1 < history_.size()) {
                ++history_index_;
                line = history_[history_index_];
            } else {
                history_index_ = history_.size();
                line.clear();
            }
            cursor = line.size();
            render_line(prompt, line, cursor);
            completion_listed = false;
            continue;
        }

        if (key == 0x104 || key == (0x100 + 75)) { // Left
            if (cursor > 0) {
                --cursor;
                render_line(prompt, line, cursor);
            }
            completion_listed = false;
            continue;
        }

        if (key == 0x103 || key == (0x100 + 77)) { // Right
            if (cursor < line.size()) {
                ++cursor;
                render_line(prompt, line, cursor);
            }
            completion_listed = false;
            continue;
        }

        if (key == 0x105 || key == (0x100 + 71)) { // Home
            cursor = 0;
            render_line(prompt, line, cursor);
            completion_listed = false;
            continue;
        }

        if (key == 0x106 || key == (0x100 + 79)) { // End
            cursor = line.size();
            render_line(prompt, line, cursor);
            completion_listed = false;
            continue;
        }

        if (key == 0x107) { // Delete
            if (cursor < line.size()) {
                line.erase(cursor, 1);
                render_line(prompt, line, cursor);
            }
            completion_listed = false;
            continue;
        }

        if (key == 0x01) { // Ctrl-A
            cursor = 0;
            render_line(prompt, line, cursor);
            completion_listed = false;
            continue;
        }

        if (key == 0x05) { // Ctrl-E
            cursor = line.size();
            render_line(prompt, line, cursor);
            completion_listed = false;
            continue;
        }

        if (key == 0x0b) { // Ctrl-K
            line.erase(cursor);
            render_line(prompt, line, cursor);
            completion_listed = false;
            continue;
        }

        if (key == 0x15) { // Ctrl-U
            line.erase(0, cursor);
            cursor = 0;
            render_line(prompt, line, cursor);
            completion_listed = false;
            continue;
        }

        if (key == '\t') {
            // Completion candidates are returned as member/builtin names.  For
            // `object.me`, only `me` is replaced; `object.` stays untouched.
            std::size_t expression_start = cursor;
            while (expression_start > 0) {
                const char ch = line[expression_start - 1];
                if (!(std::isalnum(static_cast<unsigned char>(ch)) || ch == '_' || ch == '.' || ch == '?')) break;
                --expression_start;
            }

            std::size_t token_start = expression_start;
            const std::size_t last_dot = line.rfind('.', cursor == 0 ? 0 : cursor - 1);
            if (last_dot != std::string::npos && last_dot >= expression_start && last_dot < cursor) {
                token_start = last_dot + 1;
            }

            const std::string prefix = line.substr(token_start, cursor - token_start);
            std::vector<std::string> matches;
            if (completion_listed && completion_token_start == token_start && !completion_matches.empty()) {
                matches = completion_matches;
            } else {
                const auto candidates = completer ? completer(line) : std::vector<std::string>{};
                for (const auto& candidate : candidates) {
                    if (candidate.rfind(prefix, 0) == 0) {
                        matches.push_back(candidate);
                    }
                }
                std::sort(matches.begin(), matches.end());
                matches.erase(std::unique(matches.begin(), matches.end()), matches.end());
                completion_matches = matches;
                completion_index = 0;
                completion_token_start = token_start;
            }

            if (matches.empty()) {
                completion_listed = false;
                continue;
            }

            const std::string current_prefix = line.substr(token_start, cursor - token_start);

            if (!completion_listed) {
                if (matches.size() == 1) {
                    line.replace(token_start, current_prefix.size(), matches.front());
                    cursor = token_start + matches.front().size();
                    render_line(prompt, line, cursor);
                    completion_index = 0;
                    completion_listed = true;
                    continue;
                }

                // First Tab completes the unambiguous common prefix. A second Tab
                // then starts cycling through the complete candidate set.
                std::string common = matches.front();
                for (std::size_t i = 1; i < matches.size() && !common.empty(); ++i) {
                    std::size_t n = 0;
                    while (n < common.size() && n < matches[i].size() && common[n] == matches[i][n])
                        ++n;
                    common.resize(n);
                }
                if (common.size() > current_prefix.size()) {
                    line.replace(token_start, current_prefix.size(), common);
                    cursor = token_start + common.size();
                }

                std::cout << '\n';
                for (std::size_t i = 0; i < matches.size(); ++i) {
                    std::cout << matches[i];
                    if ((i + 1) % 4 == 0 || i + 1 == matches.size()) std::cout << '\n';
                    else std::cout << "    ";
                }
                render_line(prompt, line, cursor);
                completion_listed = true;
                completion_index = 0;
                continue;
            }

            const std::string& selected = completion_matches[completion_index % completion_matches.size()];
            line.replace(token_start, current_prefix.size(), selected);
            cursor = token_start + selected.size();
            render_line(prompt, line, cursor);
            completion_index = (completion_index + 1) % completion_matches.size();
            completion_listed = true;
            continue;
        }

        if (key >= 0x20 && key <= 0x7e) {
            line.insert(cursor, 1, static_cast<char>(key));
            ++cursor;
            render_line(prompt, line, cursor);
            completion_listed = false;
        }
    }
}

} // namespace lucy

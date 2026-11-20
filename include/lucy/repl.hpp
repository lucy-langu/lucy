#pragma once

#include <functional>
#include <string>
#include <vector>

namespace lucy {

// Cross-platform interactive line editor used by the Lucy REPL.
class ReplLineEditor {
public:
    using CompletionProvider = std::function<std::vector<std::string>(const std::string&)>;

    ReplLineEditor();
    ~ReplLineEditor();

    ReplLineEditor(const ReplLineEditor&) = delete;
    ReplLineEditor& operator=(const ReplLineEditor&) = delete;

    // Reads one logical input line. Returns false on EOF/Ctrl-D.
    bool read_line(const std::string& prompt, std::string& line,
                   const CompletionProvider& completer);

    // Prints stored history entries in a compact numbered format.
    void print_history() const;

private:
    std::vector<std::string> history_;
    std::size_t history_index_ = 0;
    std::string history_path_;

    void load_history();
    void save_history_entry(const std::string& line);
    void add_history(const std::string& line);
};

} // namespace lucy

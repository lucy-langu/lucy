#include "lucy/phase2.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <ctime>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <random>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <vector>

#ifdef _WIN32
#include <process.h>
#else
#include <sys/wait.h>
#include <unistd.h>
#endif

#ifdef LUCY_HAS_SQLITE
#include <sqlite3.h>
#endif

namespace lucy {
namespace {

namespace fs = std::filesystem;

void require_count(const std::vector<Value>& args, std::size_t count, const std::string& name) {
    if (args.size() != count) {
        throw std::runtime_error(
            "ArgumentError: " + name + " expects " + std::to_string(count) +
            " argument(s), got " + std::to_string(args.size()));
    }
}

void require_range(const std::vector<Value>& args, std::size_t min, std::size_t max,
                   const std::string& name) {
    if (args.size() < min || args.size() > max) {
        throw std::runtime_error(
            "ArgumentError: " + name + " expects " + std::to_string(min) +
            " to " + std::to_string(max) + " argument(s), got " +
            std::to_string(args.size()));
    }
}

const std::string& string_arg(const Value& value, const std::string& name) {
    auto text = std::get_if<std::string>(&value.data);
    if (!text) {
        throw std::runtime_error("TypeError: " + name + " expects a string");
    }
    return *text;
}

long long integer_arg(const Value& value, const std::string& name) {
    if (auto integer = std::get_if<long long>(&value.data)) {
        return *integer;
    }
    if (auto number = std::get_if<double>(&value.data)) {
        return static_cast<long long>(*number);
    }
    throw std::runtime_error("TypeError: " + name + " expects an integer");
}

Value::ArrayPtr as_array(const Value& value, const std::string& name) {
    auto array = std::get_if<Value::ArrayPtr>(&value.data);
    if (!array) {
        throw std::runtime_error("TypeError: " + name + " expects an array");
    }
    return *array;
}

std::string read_text_file(const fs::path& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        throw std::runtime_error("IOError: cannot read '" + path.string() + "'");
    }

    std::ostringstream content;
    content << input.rdbuf();
    return content.str();
}

std::string shell_quote(const std::string& value) {
#ifdef _WIN32
    std::string result = "\"";
    for (char c : value) {
        if (c == '"') {
            result += "\\\"";
        } else {
            result += c;
        }
    }
    result += "\"";
    return result;
#else
    std::string result = "'";
    for (char c : value) {
        if (c == '\'') {
            result += "'\\''";
        } else {
            result += c;
        }
    }
    result += "'";
    return result;
#endif
}

struct CommandResult {
    std::string output;
    int status = 0;
};

CommandResult run_command(const std::string& command) {
#ifdef _WIN32
    FILE* pipe = _popen(command.c_str(), "r");
#else
    FILE* pipe = popen(command.c_str(), "r");
#endif
    if (!pipe) {
        throw std::runtime_error("ProcessError: cannot start process");
    }

    std::string output;
    char buffer[4096];
    while (std::fgets(buffer, sizeof(buffer), pipe)) {
        output += buffer;
    }

#ifdef _WIN32
    const int status = _pclose(pipe);
#else
    const int raw_status = pclose(pipe);
    const int status = WIFEXITED(raw_status) ? WEXITSTATUS(raw_status) : raw_status;
#endif

    return {output, status};
}

// ------------------------------ JSON ------------------------------

class JsonParser {
public:
    explicit JsonParser(std::string_view source) : source_(source) {}

    Value parse() {
        skip_space();
        Value result = parse_value();
        skip_space();
        if (!at_end()) {
            fail("unexpected characters after JSON value");
        }
        return result;
    }

private:
    std::string_view source_;
    std::size_t position_ = 0;

    bool at_end() const { return position_ >= source_.size(); }

    char peek() const {
        return at_end() ? '\0' : source_[position_];
    }

    char advance() {
        if (at_end()) {
            fail("unexpected end of input");
        }
        return source_[position_++];
    }

    void fail(const std::string& message) const {
        throw std::runtime_error(
            "JSONDecodeError: " + message + " at position " + std::to_string(position_));
    }

    void skip_space() {
        while (!at_end() && std::isspace(static_cast<unsigned char>(peek()))) {
            ++position_;
        }
    }

    bool consume(char expected) {
        if (peek() == expected) {
            ++position_;
            return true;
        }
        return false;
    }

    void expect(char expected) {
        if (!consume(expected)) {
            fail(std::string("expected '") + expected + "'");
        }
    }

    Value parse_value() {
        skip_space();
        switch (peek()) {
            case 'n': return parse_literal("null", Value{});
            case 't': return parse_literal("true", Value(true));
            case 'f': return parse_literal("false", Value(false));
            case '"': return Value(parse_string());
            case '[': return parse_array();
            case '{': return parse_object();
            default:
                if (peek() == '-' || std::isdigit(static_cast<unsigned char>(peek()))) {
                    return parse_number();
                }
                fail("invalid JSON value");
        }
        return Value{};
    }

    Value parse_literal(const std::string& literal, Value value) {
        if (source_.substr(position_, literal.size()) != literal) {
            fail("invalid literal");
        }
        position_ += literal.size();
        return value;
    }

    std::string parse_string() {
        expect('"');
        std::string result;

        while (!at_end()) {
            char c = advance();
            if (c == '"') {
                return result;
            }
            if (c == '\\') {
                if (at_end()) {
                    fail("unfinished escape sequence");
                }
                char escaped = advance();
                switch (escaped) {
                    case '"': result += '"'; break;
                    case '\\': result += '\\'; break;
                    case '/': result += '/'; break;
                    case 'b': result += '\b'; break;
                    case 'f': result += '\f'; break;
                    case 'n': result += '\n'; break;
                    case 'r': result += '\r'; break;
                    case 't': result += '\t'; break;
                    case 'u': parse_unicode_escape(result); break;
                    default: fail("invalid escape sequence");
                }
                continue;
            }
            if (static_cast<unsigned char>(c) < 0x20) {
                fail("control character in string");
            }
            result += c;
        }

        fail("unterminated string");
        return {};
    }

    void parse_unicode_escape(std::string& result) {
        if (position_ + 4 > source_.size()) {
            fail("incomplete unicode escape");
        }

        unsigned int codepoint = 0;
        for (int i = 0; i < 4; ++i) {
            char c = source_[position_++];
            codepoint <<= 4;
            if (c >= '0' && c <= '9') codepoint += c - '0';
            else if (c >= 'a' && c <= 'f') codepoint += c - 'a' + 10;
            else if (c >= 'A' && c <= 'F') codepoint += c - 'A' + 10;
            else fail("invalid unicode escape");
        }

        append_utf8(result, codepoint);
    }

    static void append_utf8(std::string& result, unsigned int codepoint) {
        if (codepoint <= 0x7F) {
            result += static_cast<char>(codepoint);
        } else if (codepoint <= 0x7FF) {
            result += static_cast<char>(0xC0 | (codepoint >> 6));
            result += static_cast<char>(0x80 | (codepoint & 0x3F));
        } else {
            result += static_cast<char>(0xE0 | (codepoint >> 12));
            result += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
            result += static_cast<char>(0x80 | (codepoint & 0x3F));
        }
    }

    Value parse_number() {
        const std::size_t start = position_;
        if (peek() == '-') ++position_;

        if (!std::isdigit(static_cast<unsigned char>(peek()))) {
            fail("invalid number");
        }

        if (peek() == '0') {
            ++position_;
        } else {
            while (std::isdigit(static_cast<unsigned char>(peek()))) ++position_;
        }

        bool is_double = false;
        if (consume('.')) {
            is_double = true;
            if (!std::isdigit(static_cast<unsigned char>(peek()))) {
                fail("invalid number fraction");
            }
            while (std::isdigit(static_cast<unsigned char>(peek()))) ++position_;
        }

        if (peek() == 'e' || peek() == 'E') {
            is_double = true;
            ++position_;
            if (peek() == '+' || peek() == '-') ++position_;
            if (!std::isdigit(static_cast<unsigned char>(peek()))) {
                fail("invalid number exponent");
            }
            while (std::isdigit(static_cast<unsigned char>(peek()))) ++position_;
        }

        const std::string text(source_.substr(start, position_ - start));
        try {
            return is_double ? Value(std::stod(text)) : Value(std::stoll(text));
        } catch (...) {
            fail("invalid number");
        }
        return Value{};
    }

    Value parse_array() {
        expect('[');
        Array values;
        skip_space();
        if (consume(']')) return Value(std::move(values));

        while (true) {
            values.push_back(parse_value());
            skip_space();
            if (consume(']')) break;
            expect(',');
        }
        return Value(std::move(values));
    }

    Value parse_object() {
        expect('{');
        Map values;
        skip_space();
        if (consume('}')) return Value(std::move(values));

        while (true) {
            skip_space();
            if (peek() != '"') fail("object keys must be strings");
            std::string key = parse_string();
            skip_space();
            expect(':');
            values[key] = parse_value();
            skip_space();
            if (consume('}')) break;
            expect(',');
        }
        return Value(std::move(values));
    }
};

std::string json_escape(const std::string& text) {
    std::ostringstream out;
    for (unsigned char c : text) {
        switch (c) {
            case '"': out << "\\\""; break;
            case '\\': out << "\\\\"; break;
            case '\b': out << "\\b"; break;
            case '\f': out << "\\f"; break;
            case '\n': out << "\\n"; break;
            case '\r': out << "\\r"; break;
            case '\t': out << "\\t"; break;
            default:
                if (c < 0x20) {
                    out << "\\u" << std::hex << std::setw(4) << std::setfill('0')
                        << static_cast<int>(c) << std::dec;
                } else {
                    out << static_cast<char>(c);
                }
        }
    }
    return out.str();
}

std::string json_stringify(const Value& value) {
    if (std::holds_alternative<Nil>(value.data)) return "null";
    if (auto boolean = std::get_if<bool>(&value.data)) return *boolean ? "true" : "false";
    if (auto integer = std::get_if<long long>(&value.data)) return std::to_string(*integer);
    if (auto number = std::get_if<double>(&value.data)) {
        if (!std::isfinite(*number)) {
            throw std::runtime_error("ValueError: JSON cannot represent non-finite numbers");
        }
        std::ostringstream out;
        out << std::setprecision(15) << *number;
        return out.str();
    }
    if (auto text = std::get_if<std::string>(&value.data)) {
        return "\"" + json_escape(*text) + "\"";
    }
    if (auto array = std::get_if<Value::ArrayPtr>(&value.data)) {
        std::string result = "[";
        for (std::size_t i = 0; i < (*array)->size(); ++i) {
            if (i) result += ",";
            result += json_stringify((*array)->at(i));
        }
        result += "]";
        return result;
    }
    if (auto map = std::get_if<Value::MapPtr>(&value.data)) {
        std::string result = "{";
        bool first = true;
        for (const auto& [key, item] : **map) {
            if (!first) result += ",";
            first = false;
            result += "\"" + json_escape(key) + "\":" + json_stringify(item);
        }
        result += "}";
        return result;
    }

    throw std::runtime_error("TypeError: value is not JSON serializable");
}

std::string json_pretty(const Value& value, int depth = 0) {
    const std::string indent(static_cast<std::size_t>(depth) * 2, ' ');
    const std::string child_indent(static_cast<std::size_t>(depth + 1) * 2, ' ');
    if (auto array = std::get_if<Value::ArrayPtr>(&value.data)) {
        if ((*array)->empty()) return "[]";
        std::string result = "[\n";
        for (std::size_t i = 0; i < (*array)->size(); ++i) {
            if (i) result += ",\n";
            result += child_indent + json_pretty((*array)->at(i), depth + 1);
        }
        result += "\n" + indent + "]";
        return result;
    }
    if (auto map = std::get_if<Value::MapPtr>(&value.data)) {
        if ((*map)->empty()) return "{}";
        std::string result = "{\n";
        bool first = true;
        for (const auto& [key, item] : **map) {
            if (!first) result += ",\n";
            first = false;
            result += child_indent + "\"" + json_escape(key) + "\": " + json_pretty(item, depth + 1);
        }
        result += "\n" + indent + "}";
        return result;
    }
    return json_stringify(value);
}

// ------------------------------ Encoding ------------------------------

const char* base64_table =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::string base64_encode(const std::string& input) {
    std::string result;
    int value = 0;
    int bits = -6;

    for (unsigned char c : input) {
        value = (value << 8) + c;
        bits += 8;
        while (bits >= 0) {
            result.push_back(base64_table[(value >> bits) & 0x3F]);
            bits -= 6;
        }
    }

    if (bits > -6) result.push_back(base64_table[((value << 8) >> (bits + 8)) & 0x3F]);
    while (result.size() % 4) result.push_back('=');
    return result;
}

std::string base64_decode(const std::string& input) {
    std::array<int, 256> table{};
    table.fill(-1);
    for (int i = 0; base64_table[i]; ++i) table[static_cast<unsigned char>(base64_table[i])] = i;

    std::string result;
    int value = 0;
    int bits = -8;

    for (unsigned char c : input) {
        if (c == '=') break;
        if (table[c] < 0) {
            if (std::isspace(c)) continue;
            throw std::runtime_error("ValueError: invalid base64 input");
        }
        value = (value << 6) + table[c];
        bits += 6;
        if (bits >= 0) {
            result.push_back(static_cast<char>((value >> bits) & 0xFF));
            bits -= 8;
        }
    }
    return result;
}

std::string hex_encode(const std::string& input) {
    std::ostringstream out;
    out << std::hex << std::setfill('0');
    for (unsigned char c : input) out << std::setw(2) << static_cast<int>(c);
    return out.str();
}

std::string hex_decode(const std::string& input) {
    if (input.size() % 2 != 0) {
        throw std::runtime_error("ValueError: hex input must contain an even number of digits");
    }

    auto digit = [](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return -1;
    };

    std::string result;
    for (std::size_t i = 0; i < input.size(); i += 2) {
        int high = digit(input[i]);
        int low = digit(input[i + 1]);
        if (high < 0 || low < 0) throw std::runtime_error("ValueError: invalid hex input");
        result.push_back(static_cast<char>((high << 4) | low));
    }
    return result;
}

bool is_url_safe(unsigned char c) {
    return std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~';
}

std::string url_encode(const std::string& input) {
    std::ostringstream out;
    out << std::uppercase << std::hex;
    for (unsigned char c : input) {
        if (is_url_safe(c)) out << static_cast<char>(c);
        else out << '%' << std::setw(2) << std::setfill('0') << static_cast<int>(c);
    }
    return out.str();
}

std::string url_decode(const std::string& input) {
    auto hex = [](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return -1;
    };

    std::string result;
    for (std::size_t i = 0; i < input.size(); ++i) {
        if (input[i] != '%') {
            result += input[i] == '+' ? ' ' : input[i];
            continue;
        }
        if (i + 2 >= input.size()) throw std::runtime_error("ValueError: invalid URL escape");
        int high = hex(input[i + 1]);
        int low = hex(input[i + 2]);
        if (high < 0 || low < 0) throw std::runtime_error("ValueError: invalid URL escape");
        result.push_back(static_cast<char>((high << 4) | low));
        i += 2;
    }
    return result;
}

// ------------------------------ CSV ------------------------------

Array parse_csv(const std::string& text, char delimiter) {
    Array rows;
    Array row;
    std::string field;
    bool quoted = false;

    for (std::size_t i = 0; i <= text.size(); ++i) {
        const char c = i < text.size() ? text[i] : '\n';

        if (quoted) {
            if (c == '"') {
                if (i + 1 < text.size() && text[i + 1] == '"') {
                    field += '"';
                    ++i;
                } else {
                    quoted = false;
                }
            } else {
                field += c;
            }
            continue;
        }

        if (c == '"' && field.empty()) {
            quoted = true;
        } else if (c == delimiter) {
            row.emplace_back(field);
            field.clear();
        } else if (c == '\n' || c == '\r') {
            if (c == '\r' && i + 1 < text.size() && text[i + 1] == '\n') ++i;
            row.emplace_back(field);
            field.clear();
            rows.emplace_back(std::move(row));
            row = Array{};
        } else {
            field += c;
        }
    }

    if (quoted) throw std::runtime_error("CSVError: unterminated quoted field");
    return rows;
}

std::string csv_escape(const std::string& value, char delimiter) {
    const bool needs_quotes = value.find_first_of(std::string("\"\r\n") + delimiter) != std::string::npos;
    if (!needs_quotes) return value;

    std::string result = "\"";
    for (char c : value) result += c == '"' ? "\"\"" : std::string(1, c);
    result += '"';
    return result;
}

std::string stringify_csv(const Value& value, char delimiter) {
    auto rows = as_array(value, "csv.stringify");
    std::string result;

    for (std::size_t r = 0; r < rows->size(); ++r) {
        auto row = as_array(rows->at(r), "csv.stringify");
        for (std::size_t c = 0; c < row->size(); ++c) {
            if (c) result += delimiter;
            result += csv_escape(row->at(c).to_string(), delimiter);
        }
        if (r + 1 < rows->size()) result += '\n';
    }
    return result;
}
// ------------------------------ HTTP Headers Helper ------------------------------

std::vector<std::pair<std::string, std::string>> extract_headers(
    const Value& value, const std::string& name) {
    std::vector<std::pair<std::string, std::string>> headers;

    auto map = std::get_if<Value::MapPtr>(&value.data);
    if (!map) {
        throw std::runtime_error("TypeError: " + name + " expects a map for headers");
    }

    for (const auto& [key, item] : **map) {
        std::string header_value;
        if (auto text = std::get_if<std::string>(&item.data)) {
            header_value = *text;
        } else if (auto integer = std::get_if<long long>(&item.data)) {
            header_value = std::to_string(*integer);
        } else if (auto number = std::get_if<double>(&item.data)) {
            header_value = std::to_string(*number);
        } else if (auto boolean = std::get_if<bool>(&item.data)) {
            header_value = *boolean ? "true" : "false";
        } else {
            throw std::runtime_error(
                "TypeError: " + name + " header '" + key + "' must be a scalar value");
        }
        headers.emplace_back(key, header_value);
    }

    return headers;
}

// ------------------------------ HTTP ------------------------------
Value http_request(const std::vector<Value>& args, const std::string& method) {
    require_range(args, 1, 5, "http." + method);
    const std::string url = string_arg(args[0], "http." + method);

    std::string body;
    std::vector<std::pair<std::string, std::string>> headers;
    Value::MapPtr options;

    // arg 2: body or headers
    if (args.size() >= 2) {
        if (std::holds_alternative<Nil>(args[1].data)) {
            // nil = no body
        } else if (std::holds_alternative<Value::MapPtr>(args[1].data)) {
            headers = extract_headers(args[1], "http." + method);
        } else {
            body = string_arg(args[1], "http." + method);
        }
    }

    // arg 3: headers when arg 2 is the body
    if (args.size() >= 3 && std::holds_alternative<Value::MapPtr>(args[2].data)) {
        headers = extract_headers(args[2], "http." + method);
    }

    // arg 4: explicit headers (kept for compatibility with the existing API)
    if (args.size() >= 4 && !std::holds_alternative<Nil>(args[3].data)) {
        headers = extract_headers(args[3], "http." + method);
    }

    // arg 5: request options
    if (args.size() >= 5) {
        auto map = std::get_if<Value::MapPtr>(&args[4].data);
        if (!map) throw std::runtime_error("TypeError: http.request options must be a map");
        options = *map;
    }

    int timeout = 30;
    int connect_timeout = 0;
    std::string proxy;
    std::string user_agent;
    bool follow_redirects = true;
    bool insecure = false;

    if (options) {
        if (auto it = options->find("timeout"); it != options->end())
            timeout = static_cast<int>(integer_arg(it->second, "http.timeout"));
        if (auto it = options->find("connect_timeout"); it != options->end())
            connect_timeout = static_cast<int>(integer_arg(it->second, "http.connect_timeout"));
        if (auto it = options->find("proxy"); it != options->end())
            proxy = string_arg(it->second, "http.proxy");
        if (auto it = options->find("user_agent"); it != options->end())
            user_agent = string_arg(it->second, "http.user_agent");
        if (auto it = options->find("follow_redirects"); it != options->end()) {
            if (auto value = std::get_if<bool>(&it->second.data)) follow_redirects = *value;
            else throw std::runtime_error("TypeError: http.follow_redirects expects a boolean");
        }
        if (auto it = options->find("insecure"); it != options->end()) {
            if (auto value = std::get_if<bool>(&it->second.data)) insecure = *value;
            else throw std::runtime_error("TypeError: http.insecure expects a boolean");
        }
    }

    if (timeout <= 0) throw std::runtime_error("ValueError: http.timeout must be greater than zero");
    if (connect_timeout < 0) throw std::runtime_error("ValueError: http.connect_timeout cannot be negative");

    fs::path temp = fs::temp_directory_path() /
        ("lucy-http-" + std::to_string(std::random_device{}()));
    fs::path output_path = temp.string() + ".body";
    fs::path header_path = temp.string() + ".headers";
    fs::path data_path = temp.string() + ".data";

    struct Cleanup {
        fs::path a, b, c;
        ~Cleanup() {
            std::error_code ec;
            fs::remove(a, ec);
            fs::remove(b, ec);
            fs::remove(c, ec);
        }
    } cleanup{output_path, header_path, data_path};

    if (!body.empty()) {
        std::ofstream data(data_path, std::ios::binary);
        if (!data) throw std::runtime_error("HTTPError: cannot create temporary request body");
        data << body;
    }

    std::string command = "curl --silent --show-error --max-time " + std::to_string(timeout);
    command += " --request " + shell_quote(method);
    if (follow_redirects) command += " --location";
    if (connect_timeout > 0) command += " --connect-timeout " + std::to_string(connect_timeout);
    if (!proxy.empty()) command += " --proxy " + shell_quote(proxy);
    if (!user_agent.empty()) command += " --user-agent " + shell_quote(user_agent);
    if (insecure) command += " --insecure";

    for (const auto& [key, value] : headers)
        command += " --header " + shell_quote(key + ": " + value);

    command += " --dump-header " + shell_quote(header_path.string());
    command += " --output " + shell_quote(output_path.string());
    if (!body.empty()) command += " --data-binary " + shell_quote("@" + data_path.string());
    command += " --write-out " + shell_quote("%{http_code}");
    command += " " + shell_quote(url);

    CommandResult result = run_command(command);
    if (result.status != 0)
        throw std::runtime_error("HTTPError: curl failed with status " + std::to_string(result.status));

    int status_code = 0;
    try {
        status_code = std::stoi(result.output);
    } catch (...) {
        throw std::runtime_error("HTTPError: invalid HTTP status from curl");
    }

    Map response;
    response["status"] = static_cast<long long>(status_code);
    response["body"] = read_text_file(output_path);
    response["headers"] = Map{};

    auto response_headers = std::get<Value::MapPtr>(response["headers"].data);
    std::ifstream header_file(header_path);
    std::string line;
    while (std::getline(header_file, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        const auto colon = line.find(':');
        if (colon == std::string::npos) continue;
        std::string key = line.substr(0, colon);
        std::string value = line.substr(colon + 1);
        while (!value.empty() && std::isspace(static_cast<unsigned char>(value.front()))) value.erase(value.begin());
        std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        (*response_headers)[key] = value;
    }

    response["ok"] = status_code >= 200 && status_code < 300;
    return Value(std::move(response));
}

#ifdef LUCY_HAS_SQLITE
Value sqlite_query(const std::string& database_path, const std::string& sql) {
    sqlite3* db = nullptr;
    if (sqlite3_open(database_path.c_str(), &db) != SQLITE_OK) {
        std::string message = db ? sqlite3_errmsg(db) : "cannot open database";
        if (db) sqlite3_close(db);
        throw std::runtime_error("SQLiteError: " + message);
    }

    sqlite3_stmt* statement = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &statement, nullptr) != SQLITE_OK) {
        std::string message = sqlite3_errmsg(db);
        sqlite3_close(db);
        throw std::runtime_error("SQLiteError: " + message);
    }

    Array rows;
    const int column_count = sqlite3_column_count(statement);

    while (true) {
        const int result = sqlite3_step(statement);
        if (result == SQLITE_DONE) break;
        if (result != SQLITE_ROW) {
            std::string message = sqlite3_errmsg(db);
            sqlite3_finalize(statement);
            sqlite3_close(db);
            throw std::runtime_error("SQLiteError: " + message);
        }

        Map row;
        for (int i = 0; i < column_count; ++i) {
            const char* name = sqlite3_column_name(statement, i);
            switch (sqlite3_column_type(statement, i)) {
                case SQLITE_INTEGER:
                    row[name] = static_cast<long long>(sqlite3_column_int64(statement, i));
                    break;
                case SQLITE_FLOAT:
                    row[name] = sqlite3_column_double(statement, i);
                    break;
                case SQLITE_TEXT:
                    row[name] = std::string(reinterpret_cast<const char*>(sqlite3_column_text(statement, i)));
                    break;
                case SQLITE_NULL:
                    row[name] = Value{};
                    break;
                case SQLITE_BLOB: {
                    const auto* data = static_cast<const unsigned char*>(sqlite3_column_blob(statement, i));
                    const int size = sqlite3_column_bytes(statement, i);
                    row[name] = hex_encode(std::string(reinterpret_cast<const char*>(data), size));
                    break;
                }
            }
        }
        rows.emplace_back(std::move(row));
    }

    sqlite3_finalize(statement);
    sqlite3_close(db);
    return Value(std::move(rows));
}
#endif

} // namespace

void install_phase2_builtins(BuiltinMap& builtins) {
    // Regex ---------------------------------------------------------
    builtins["__regex_match"] = [](const std::vector<Value>& args) {
        require_count(args, 2, "regex.match");
        try {
            return Value(std::regex_match(string_arg(args[1], "regex.match"),
                                          std::regex(string_arg(args[0], "regex.match"))));
        } catch (const std::regex_error& error) {
            throw std::runtime_error(std::string("RegexError: ") + error.what());
        }
    };

    builtins["__regex_search"] = [](const std::vector<Value>& args) {
        require_count(args, 2, "regex.search");
        try {
            return Value(std::regex_search(string_arg(args[1], "regex.search"),
                                           std::regex(string_arg(args[0], "regex.search"))));
        } catch (const std::regex_error& error) {
            throw std::runtime_error(std::string("RegexError: ") + error.what());
        }
    };

    builtins["__regex_find_all"] = [](const std::vector<Value>& args) {
        require_count(args, 2, "regex.find_all");
        try {
            std::regex pattern(string_arg(args[0], "regex.find_all"));
            std::string text = string_arg(args[1], "regex.find_all");
            Array result;
            for (std::sregex_iterator it(text.begin(), text.end(), pattern), end; it != end; ++it) {
                result.emplace_back(it->str());
            }
            return Value(std::move(result));
        } catch (const std::regex_error& error) {
            throw std::runtime_error(std::string("RegexError: ") + error.what());
        }
    };

    builtins["__regex_replace"] = [](const std::vector<Value>& args) {
        require_count(args, 3, "regex.replace");
        try {
            return Value(std::regex_replace(string_arg(args[2], "regex.replace"),
                                             std::regex(string_arg(args[0], "regex.replace")),
                                             string_arg(args[1], "regex.replace")));
        } catch (const std::regex_error& error) {
            throw std::runtime_error(std::string("RegexError: ") + error.what());
        }
    };

    // JSON ----------------------------------------------------------
    builtins["__json_parse"] = [](const std::vector<Value>& args) {
        require_count(args, 1, "json.parse");
        return JsonParser(string_arg(args[0], "json.parse")).parse();
    };

    builtins["__json_stringify"] = [](const std::vector<Value>& args) {
        require_count(args, 1, "json.stringify");
        return Value(json_stringify(args[0]));
    };

    // Date and time -------------------------------------------------
    builtins["__datetime_now"] = [](const std::vector<Value>& args) {
        require_count(args, 0, "datetime.now");
        const auto now = std::chrono::system_clock::now();
        const auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();
        return Value(static_cast<long long>(milliseconds));
    };

    builtins["__datetime_from_timestamp"] = [](const std::vector<Value>& args) {
        require_count(args, 1, "datetime.from_timestamp");
        return Value(integer_arg(args[0], "datetime.from_timestamp"));
    };

    builtins["__datetime_format"] = [](const std::vector<Value>& args) {
        require_count(args, 2, "datetime.format");
        const auto milliseconds = integer_arg(args[0], "datetime.format");
        const std::string format = string_arg(args[1], "datetime.format");
        const auto time = std::chrono::system_clock::time_point(std::chrono::milliseconds(milliseconds));
        const std::time_t raw = std::chrono::system_clock::to_time_t(time);
        std::tm local{};
#ifdef _WIN32
        localtime_s(&local, &raw);
#else
        localtime_r(&raw, &local);
#endif
        std::ostringstream output;
        output << std::put_time(&local, format.c_str());
        return Value(output.str());
    };

    builtins["__datetime_parts"] = [](const std::vector<Value>& args) {
        require_count(args, 1, "datetime.parts");
        const auto milliseconds = integer_arg(args[0], "datetime.parts");
        const auto time = std::chrono::system_clock::time_point(std::chrono::milliseconds(milliseconds));
        const std::time_t raw = std::chrono::system_clock::to_time_t(time);
        std::tm local{};
#ifdef _WIN32
        localtime_s(&local, &raw);
#else
        localtime_r(&raw, &local);
#endif
        Map parts;
        parts["year"] = static_cast<long long>(local.tm_year + 1900);
        parts["month"] = static_cast<long long>(local.tm_mon + 1);
        parts["day"] = static_cast<long long>(local.tm_mday);
        parts["hour"] = static_cast<long long>(local.tm_hour);
        parts["minute"] = static_cast<long long>(local.tm_min);
        parts["second"] = static_cast<long long>(local.tm_sec);
        parts["weekday"] = static_cast<long long>(local.tm_wday);
        return Value(std::move(parts));
    };

    // Process -------------------------------------------------------
    builtins["__process_run"] = [](const std::vector<Value>& args) {
        require_count(args, 1, "process.run");
        CommandResult result = run_command(string_arg(args[0], "process.run"));
        Map value;
        value["stdout"] = result.output;
        value["status"] = static_cast<long long>(result.status);
        value["success"] = result.status == 0;
        return Value(std::move(value));
    };

    builtins["__process_capture"] = [](const std::vector<Value>& args) {
        require_count(args, 1, "process.capture");
        CommandResult result = run_command(string_arg(args[0], "process.capture"));
        if (result.status != 0) {
            throw std::runtime_error("ProcessError: process exited with status " + std::to_string(result.status));
        }
        return Value(result.output);
    };

    // Encoding ------------------------------------------------------
    builtins["__encoding_base64_encode"] = [](const std::vector<Value>& args) {
        require_count(args, 1, "encoding.base64_encode");
        return Value(base64_encode(string_arg(args[0], "encoding.base64_encode")));
    };

    builtins["__encoding_base64_decode"] = [](const std::vector<Value>& args) {
        require_count(args, 1, "encoding.base64_decode");
        return Value(base64_decode(string_arg(args[0], "encoding.base64_decode")));
    };

    builtins["__encoding_hex_encode"] = [](const std::vector<Value>& args) {
        require_count(args, 1, "encoding.hex_encode");
        return Value(hex_encode(string_arg(args[0], "encoding.hex_encode")));
    };

    builtins["__encoding_hex_decode"] = [](const std::vector<Value>& args) {
        require_count(args, 1, "encoding.hex_decode");
        return Value(hex_decode(string_arg(args[0], "encoding.hex_decode")));
    };

    builtins["__encoding_url_encode"] = [](const std::vector<Value>& args) {
        require_count(args, 1, "encoding.url_encode");
        return Value(url_encode(string_arg(args[0], "encoding.url_encode")));
    };

    builtins["__encoding_url_decode"] = [](const std::vector<Value>& args) {
        require_count(args, 1, "encoding.url_decode");
        return Value(url_decode(string_arg(args[0], "encoding.url_decode")));
    };

    // CSV -----------------------------------------------------------
    builtins["__csv_parse"] = [](const std::vector<Value>& args) {
        require_range(args, 1, 2, "csv.parse");
        const char delimiter = args.size() == 2 ? string_arg(args[1], "csv.parse")[0] : ',';
        return Value(parse_csv(string_arg(args[0], "csv.parse"), delimiter));
    };

    builtins["__csv_stringify"] = [](const std::vector<Value>& args) {
        require_range(args, 1, 2, "csv.stringify");
        const char delimiter = args.size() == 2 ? string_arg(args[1], "csv.stringify")[0] : ',';
        return Value(stringify_csv(args[0], delimiter));
    };

    // HTTP ----------------------------------------------------------
    builtins["__http_get"] = [](const std::vector<Value>& args) {
        return http_request(args, "GET");
    };

    builtins["__http_post"] = [](const std::vector<Value>& args) {
        return http_request(args, "POST");
    };
    builtins["__http_put"] = [](const std::vector<Value>& args) {
        return http_request(args, "PUT");
    };
    builtins["__http_delete"] = [](const std::vector<Value>& args) {
        return http_request(args, "DELETE");
    };
    builtins["__http_head"] = [](const std::vector<Value>& args) {
        return http_request(args, "HEAD");
    };
    builtins["__http_patch"] = [](const std::vector<Value>& args) {
        return http_request(args, "PATCH");
    };

    builtins["__http_request"] = [](const std::vector<Value>& args) {
        require_range(args, 2, 5, "http.request");
        const std::string method = string_arg(args[0], "http.request");
        std::vector<Value> request_args;
        request_args.push_back(args[1]);
        if (args.size() >= 3) request_args.push_back(args[2]);
        if (args.size() >= 4) request_args.push_back(args[3]);
        if (args.size() >= 5) request_args.push_back(args[4]);
        return http_request(request_args, method);
    };

    builtins["__json_pretty"] = [](const std::vector<Value>& args) {
        require_count(args, 1, "json.pretty_generate");
        return Value(json_pretty(args[0]));
    };

    // SQLite --------------------------------------------------------
#ifdef LUCY_HAS_SQLITE
    builtins["__sqlite_execute"] = [](const std::vector<Value>& args) {
        require_count(args, 2, "sqlite.execute");
        const std::string database_path = string_arg(args[0], "sqlite.execute");
        const std::string sql = string_arg(args[1], "sqlite.execute");

        sqlite3* db = nullptr;
        if (sqlite3_open(database_path.c_str(), &db) != SQLITE_OK) {
            std::string message = db ? sqlite3_errmsg(db) : "cannot open database";
            if (db) sqlite3_close(db);
            throw std::runtime_error("SQLiteError: " + message);
        }

        char* error = nullptr;
        if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &error) != SQLITE_OK) {
            std::string message = error ? error : sqlite3_errmsg(db);
            sqlite3_free(error);
            sqlite3_close(db);
            throw std::runtime_error("SQLiteError: " + message);
        }

        const long long changes = sqlite3_changes64(db);
        sqlite3_close(db);
        return Value(changes);
    };

    builtins["__sqlite_query"] = [](const std::vector<Value>& args) {
        require_count(args, 2, "sqlite.query");
        return sqlite_query(string_arg(args[0], "sqlite.query"), string_arg(args[1], "sqlite.query"));
    };
#else
    builtins["__sqlite_execute"] = [](const std::vector<Value>&) -> Value {
        throw std::runtime_error("SQLiteError: Lucy was built without SQLite support");
    };
    builtins["__sqlite_query"] = [](const std::vector<Value>&) -> Value {
        throw std::runtime_error("SQLiteError: Lucy was built without SQLite support");
    };
#endif
}

} // namespace lucy

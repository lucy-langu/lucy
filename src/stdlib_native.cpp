#include "lucy/stdlib_native.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cctype>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <regex>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <cstdint>
#include <vector>

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#include <process.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#ifndef NOMINMAX
#define NOMINMAX
#endif
#else
#include <arpa/inet.h>
#include <dirent.h>
#include <fcntl.h>
#include <grp.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pwd.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

namespace lucy {
namespace {

namespace fs = std::filesystem;

void require_count(const std::vector<Value>& args, std::size_t count, const std::string& name) {
    if (args.size() != count) {
        throw std::runtime_error("ArgumentError: " + name + " expects " +
                                 std::to_string(count) + " argument(s), got " +
                                 std::to_string(args.size()));
    }
}

void require_range(const std::vector<Value>& args, std::size_t minimum,
                   std::size_t maximum, const std::string& name) {
    if (args.size() < minimum || args.size() > maximum) {
        throw std::runtime_error("ArgumentError: " + name + " expects " +
                                 std::to_string(minimum) + " to " +
                                 std::to_string(maximum) + " argument(s), got " +
                                 std::to_string(args.size()));
    }
}

const std::string& string_arg(const Value& value, const std::string& name) {
    auto text = std::get_if<std::string>(&value.data);
    if (!text) throw std::runtime_error("TypeError: " + name + " expects a string");
    return *text;
}

long long integer_arg(const Value& value, const std::string& name) {
    if (auto integer = std::get_if<long long>(&value.data)) return *integer;
    if (auto number = std::get_if<double>(&value.data)) return static_cast<long long>(*number);
    throw std::runtime_error("TypeError: " + name + " expects a number");
}

Value::MapPtr map_arg(const Value& value, const std::string& name) {
    auto map = std::get_if<Value::MapPtr>(&value.data);
    if (!map) throw std::runtime_error("TypeError: " + name + " expects a map");
    return *map;
}

Value::ArrayPtr array_arg(const Value& value, const std::string& name) {
    auto array = std::get_if<Value::ArrayPtr>(&value.data);
    if (!array) throw std::runtime_error("TypeError: " + name + " expects an array");
    return *array;
}

std::string hex_encode(const std::string& input) {
    std::ostringstream out;
    out << std::hex << std::setfill('0');
    for (unsigned char c : input) out << std::setw(2) << static_cast<int>(c);
    return out.str();
}

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
            result.push_back(base64_table[(value >> bits) & 0x3f]);
            bits -= 6;
        }
    }
    if (bits > -6) result.push_back(base64_table[((value << 8) >> (bits + 8)) & 0x3f]);
    while (result.size() % 4) result.push_back('=');
    return result;
}

// Small self-contained SHA-256 implementation used by Digest and HMAC.
class Sha256 {
public:
    Sha256() { reset(); }

    void reset() {
        state_ = {0x6a09e667u, 0xbb67ae85u, 0x3c6ef372u, 0xa54ff53au,
                  0x510e527fu, 0x9b05688cu, 0x1f83d9abu, 0x5be0cd19u};
        bit_count_ = 0;
        buffer_size_ = 0;
    }

    void update(const std::string& data) {
        update(reinterpret_cast<const unsigned char*>(data.data()), data.size());
    }

    std::string final() {
        const std::uint64_t original_bits = bit_count_;
        unsigned char one = 0x80;
        update(&one, 1);
        unsigned char zero = 0;
        while (buffer_size_ != 56) update(&zero, 1);

        unsigned char length[8];
        for (int i = 0; i < 8; ++i)
            length[7 - i] = static_cast<unsigned char>(original_bits >> (i * 8));
        update(length, sizeof(length));

        std::string result(32, '\0');
        for (std::size_t i = 0; i < state_.size(); ++i) {
            result[i * 4 + 0] = static_cast<char>(state_[i] >> 24);
            result[i * 4 + 1] = static_cast<char>(state_[i] >> 16);
            result[i * 4 + 2] = static_cast<char>(state_[i] >> 8);
            result[i * 4 + 3] = static_cast<char>(state_[i]);
        }
        return result;
    }

private:
    static std::uint32_t rotr(std::uint32_t x, std::uint32_t n) {
        return (x >> n) | (x << (32 - n));
    }

    void transform(const unsigned char block[64]) {
        static constexpr std::uint32_t k[64] = {
            0x428a2f98u, 0x71374491u, 0xb5c0fbcfu, 0xe9b5dba5u,
            0x3956c25bu, 0x59f111f1u, 0x923f82a4u, 0xab1c5ed5u,
            0xd807aa98u, 0x12835b01u, 0x243185beu, 0x550c7dc3u,
            0x72be5d74u, 0x80deb1feu, 0x9bdc06a7u, 0xc19bf174u,
            0xe49b69c1u, 0xefbe4786u, 0x0fc19dc6u, 0x240ca1ccu,
            0x2de92c6fu, 0x4a7484aau, 0x5cb0a9dcu, 0x76f988dau,
            0x983e5152u, 0xa831c66du, 0xb00327c8u, 0xbf597fc7u,
            0xc6e00bf3u, 0xd5a79147u, 0x06ca6351u, 0x14292967u,
            0x27b70a85u, 0x2e1b2138u, 0x4d2c6dfcu, 0x53380d13u,
            0x650a7354u, 0x766a0abbu, 0x81c2c92eu, 0x92722c85u,
            0xa2bfe8a1u, 0xa81a664bu, 0xc24b8b70u, 0xc76c51a3u,
            0xd192e819u, 0xd6990624u, 0xf40e3585u, 0x106aa070u,
            0x19a4c116u, 0x1e376c08u, 0x2748774cu, 0x34b0bcb5u,
            0x391c0cb3u, 0x4ed8aa4au, 0x5b9cca4fu, 0x682e6ff3u,
            0x748f82eeu, 0x78a5636fu, 0x84c87814u, 0x8cc70208u,
            0x90befffau, 0xa4506cebu, 0xbef9a3f7u, 0xc67178f2u};
        std::uint32_t w[64]{};
        for (int i = 0; i < 16; ++i) {
            w[i] = (static_cast<std::uint32_t>(block[i * 4]) << 24) |
                   (static_cast<std::uint32_t>(block[i * 4 + 1]) << 16) |
                   (static_cast<std::uint32_t>(block[i * 4 + 2]) << 8) |
                   static_cast<std::uint32_t>(block[i * 4 + 3]);
        }
        for (int i = 16; i < 64; ++i) {
            const auto s0 = rotr(w[i - 15], 7) ^ rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
            const auto s1 = rotr(w[i - 2], 17) ^ rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
            w[i] = w[i - 16] + s0 + w[i - 7] + s1;
        }
        std::uint32_t a = state_[0], b = state_[1], c = state_[2], d = state_[3];
        std::uint32_t e = state_[4], f = state_[5], g = state_[6], h = state_[7];
        for (int i = 0; i < 64; ++i) {
            const auto s1 = rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25);
            const auto choose = (e & f) ^ ((~e) & g);
            const auto temp1 = h + s1 + choose + k[i] + w[i];
            const auto s0 = rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22);
            const auto majority = (a & b) ^ (a & c) ^ (b & c);
            const auto temp2 = s0 + majority;
            h = g; g = f; f = e; e = d + temp1;
            d = c; c = b; b = a; a = temp1 + temp2;
        }
        state_[0] += a; state_[1] += b; state_[2] += c; state_[3] += d;
        state_[4] += e; state_[5] += f; state_[6] += g; state_[7] += h;
    }

    void update(const unsigned char* data, std::size_t size) {
        bit_count_ += static_cast<std::uint64_t>(size) * 8;
        while (size > 0) {
            const std::size_t take = std::min(size, 64 - buffer_size_);
            std::memcpy(buffer_.data() + buffer_size_, data, take);
            buffer_size_ += take;
            data += take;
            size -= take;
            if (buffer_size_ == 64) {
                transform(buffer_.data());
                buffer_size_ = 0;
            }
        }
    }

    std::array<std::uint32_t, 8> state_{};
    std::array<unsigned char, 64> buffer_{};
    std::uint64_t bit_count_ = 0;
    std::size_t buffer_size_ = 0;
};

std::string sha256(const std::string& text) {
    Sha256 hash;
    hash.update(text);
    return hash.final();
}

std::string hmac_sha256(const std::string& key, const std::string& text) {
    std::string normalized = key;
    if (normalized.size() > 64) normalized = sha256(normalized);
    normalized.resize(64, '\0');
    std::string inner(64, '\x36');
    std::string outer(64, '\x5c');
    for (std::size_t i = 0; i < 64; ++i) {
        inner[i] ^= normalized[i];
        outer[i] ^= normalized[i];
    }
    Sha256 inner_hash;
    inner_hash.update(inner);
    inner_hash.update(text);
    const std::string inner_digest = inner_hash.final();
    Sha256 outer_hash;
    outer_hash.update(outer);
    outer_hash.update(inner_digest);
    return outer_hash.final();
}

Value digest_value(const std::string& text, bool raw, bool base64) {
    const std::string digest = sha256(text);
    if (raw) return Value(digest);
    if (base64) return Value(base64_encode(digest));
    return Value(hex_encode(digest));
}

std::string read_binary(const std::string& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) throw std::runtime_error("IOError: cannot read '" + path + "'");
    std::ostringstream out;
    out << input.rdbuf();
    return out.str();
}

std::string shell_quote(const std::string& value) {
#ifdef _WIN32
    std::string result = "\"";
    for (char c : value) {
        if (c == '"') result += "\\\"";
        else result += c;
    }
    result += '"';
    return result;
#else
    std::string result = "'";
    for (char c : value) {
        if (c == '\'') result += "'\\''";
        else result += c;
    }
    result += '\'';
    return result;
#endif
}

struct CommandResult { std::string output; int status = 0; };

CommandResult run_command(const std::string& command) {
#ifdef _WIN32
    FILE* pipe = _popen(command.c_str(), "r");
#else
    FILE* pipe = popen(command.c_str(), "r");
#endif
    if (!pipe) throw std::runtime_error("ProcessError: cannot start process");
    std::string output;
    char buffer[4096];
    while (std::fgets(buffer, sizeof(buffer), pipe)) output += buffer;
#ifdef _WIN32
    const int status = _pclose(pipe);
#else
    const int raw = pclose(pipe);
    const int status = WIFEXITED(raw) ? WEXITSTATUS(raw) : raw;
#endif
    return {output, status};
}

std::string format_values(const std::string& format, const std::vector<Value>& values) {
    std::ostringstream out;
    std::size_t index = 0;
    for (std::size_t i = 0; i < format.size(); ++i) {
        if (format[i] != '%' || i + 1 >= format.size()) {
            out << format[i];
            continue;
        }
        const char spec = format[++i];
        if (spec == '%') { out << '%'; continue; }
        if (index >= values.size()) throw std::runtime_error("ArgumentError: not enough format arguments");
        const Value& value = values[index++];
        if (spec == 's' || spec == 'd' || spec == 'i' || spec == 'f') out << value.to_string();
        else throw std::runtime_error(std::string("FormatError: unsupported format specifier %") + spec);
    }
    if (index != values.size()) throw std::runtime_error("ArgumentError: too many format arguments");
    return out.str();
}

Value make_file_descriptor(const std::string& path, const std::string& mode) {
    Map descriptor;
    descriptor["path"] = path;
    descriptor["mode"] = mode;
    descriptor["pos"] = 0LL;
    descriptor["closed"] = false;
    return Value(std::move(descriptor));
}

Value::MapPtr descriptor_arg(const Value& value, const std::string& name) {
    return map_arg(value, name);
}

void check_descriptor(const Value::MapPtr& descriptor, const std::string& name) {
    auto it = descriptor->find("closed");
    if (it != descriptor->end() && it->second.is_truthy())
        throw std::runtime_error("IOError: " + name + " on closed IO");
}

long long descriptor_position(const Value::MapPtr& descriptor) {
    auto it = descriptor->find("pos");
    return it == descriptor->end() ? 0 : integer_arg(it->second, "IO.pos");
}

void set_descriptor_position(const Value::MapPtr& descriptor, long long pos) {
    (*descriptor)["pos"] = pos;
}

// ------------------------------ sockets ------------------------------
#ifdef _WIN32
using SocketHandle = SOCKET;
constexpr SocketHandle invalid_socket = INVALID_SOCKET;
#else
using SocketHandle = int;
constexpr SocketHandle invalid_socket = -1;
#endif

void ensure_socket_runtime() {
#ifdef _WIN32
    static bool initialized = false;
    if (!initialized) {
        WSADATA data{};
        if (WSAStartup(MAKEWORD(2, 2), &data) != 0)
            throw std::runtime_error("SocketError: WSAStartup failed");
        initialized = true;
    }
#endif
}

SocketHandle socket_from_value(const Value& value, const std::string& name) {
    auto map = map_arg(value, name);
    auto it = map->find("handle");
    if (it == map->end()) throw std::runtime_error("SocketError: invalid socket object");
    return static_cast<SocketHandle>(integer_arg(it->second, name));
}

Value socket_value(SocketHandle handle) {
    Map result;
    result["handle"] = static_cast<long long>(handle);
    result["closed"] = false;
    return Value(std::move(result));
}

void close_socket(SocketHandle handle) {
#ifdef _WIN32
    closesocket(handle);
#else
    close(handle);
#endif
}

std::string socket_error() {
#ifdef _WIN32
    return std::to_string(WSAGetLastError());
#else
    return std::strerror(errno);
#endif
}

// ------------------------------ shellwords ------------------------------
std::vector<std::string> shell_split(const std::string& text) {
    std::vector<std::string> result;
    std::string current;
    bool single = false, double_quote = false, escaped = false;
    for (char c : text) {
        if (escaped) { current += c; escaped = false; continue; }
        if (c == '\\' && !single) { escaped = true; continue; }
        if (c == '\'' && !double_quote) { single = !single; continue; }
        if (c == '"' && !single) { double_quote = !double_quote; continue; }
        if (std::isspace(static_cast<unsigned char>(c)) && !single && !double_quote) {
            if (!current.empty()) { result.push_back(current); current.clear(); }
        } else current += c;
    }
    if (escaped || single || double_quote)
        throw std::runtime_error("ShellwordsError: unterminated quote or escape");
    if (!current.empty()) result.push_back(current);
    return result;
}

std::string shell_escape(const std::string& text) {
    return shell_quote(text);
}

std::string trim_copy(const std::string& text) {
    std::size_t begin = 0;
    while (begin < text.size() && std::isspace(static_cast<unsigned char>(text[begin]))) ++begin;
    std::size_t end = text.size();
    while (end > begin && std::isspace(static_cast<unsigned char>(text[end - 1]))) --end;
    return text.substr(begin, end - begin);
}

Value yaml_scalar(const std::string& text) {
    const std::string value = trim_copy(text);
    if (value == "null" || value == "~") return Value{};
    if (value == "true") return Value(true);
    if (value == "false") return Value(false);
    if (value.size() >= 2 && ((value.front() == '"' && value.back() == '"') ||
                              (value.front() == '\'' && value.back() == '\''))) {
        return Value(value.substr(1, value.size() - 2));
    }
    try {
        std::size_t consumed = 0;
        const long long integer = std::stoll(value, &consumed);
        if (consumed == value.size()) return Value(integer);
    } catch (...) {}
    try {
        std::size_t consumed = 0;
        const double number = std::stod(value, &consumed);
        if (consumed == value.size()) return Value(number);
    } catch (...) {}
    return Value(value);
}

Value yaml_load_simple(const std::string& source) {
    // This parser intentionally covers the common YAML subset used by configuration files:
    // mappings, sequences, strings, numbers, booleans, and null values.
    std::vector<std::pair<int, std::string>> lines;
    std::istringstream input(source);
    std::string line;
    while (std::getline(input, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        const std::size_t comment = line.find('#');
        if (comment != std::string::npos) line.erase(comment);
        const std::string content = trim_copy(line);
        if (content.empty()) continue;
        const int indent = static_cast<int>(line.find_first_not_of(' '));
        lines.emplace_back(indent < 0 ? 0 : indent, content);
    }
    if (lines.empty()) return Value(Map{});

    std::function<Value(std::size_t&, int)> parse_block = [&](std::size_t& index, int indent) -> Value {
        if (index >= lines.size()) return Value{};
        const bool sequence = lines[index].second.rfind("- ", 0) == 0 || lines[index].second == "-";
        if (sequence) {
            Array result;
            while (index < lines.size() && lines[index].first == indent) {
                const std::string item = lines[index].second;
                if (item.rfind("- ", 0) != 0 && item != "-") break;
                const std::string rest = item == "-" ? "" : trim_copy(item.substr(2));
                ++index;
                if (rest.empty() && index < lines.size() && lines[index].first > indent)
                    result.push_back(parse_block(index, lines[index].first));
                else
                    result.push_back(yaml_scalar(rest));
            }
            return Value(std::move(result));
        }

        Map result;
        while (index < lines.size() && lines[index].first == indent) {
            const std::string item = lines[index].second;
            const std::size_t colon = item.find(':');
            if (colon == std::string::npos) {
                ++index;
                continue;
            }
            const std::string key = trim_copy(item.substr(0, colon));
            const std::string rest = trim_copy(item.substr(colon + 1));
            ++index;
            if (rest.empty() && index < lines.size() && lines[index].first > indent)
                result[key] = parse_block(index, lines[index].first);
            else
                result[key] = yaml_scalar(rest);
        }
        return Value(std::move(result));
    };

    std::size_t index = 0;
    return parse_block(index, lines[0].first);
}

std::string yaml_dump_scalar(const Value& value) {
    if (std::holds_alternative<Nil>(value.data)) return "null";
    if (auto p = std::get_if<bool>(&value.data)) return *p ? "true" : "false";
    if (auto p = std::get_if<long long>(&value.data)) return std::to_string(*p);
    if (auto p = std::get_if<double>(&value.data)) { std::ostringstream out; out << *p; return out.str(); }
    if (auto p = std::get_if<std::string>(&value.data)) {
        if (p->empty() || p->find_first_of(" #:[]{}\n\t\"'") != std::string::npos)
            return "\"" + *p + "\"";
        return *p;
    }
    return value.to_string();
}

void yaml_dump_value(const Value& value, std::ostringstream& out, int indent) {
    const std::string padding(static_cast<std::size_t>(indent), ' ');
    if (auto map = std::get_if<Value::MapPtr>(&value.data)) {
        for (const auto& [key, item] : **map) {
            out << padding << key << ":";
            if (std::holds_alternative<Value::MapPtr>(item.data) || std::holds_alternative<Value::ArrayPtr>(item.data)) {
                out << "\n";
                yaml_dump_value(item, out, indent + 2);
            } else {
                out << " " << yaml_dump_scalar(item) << "\n";
            }
        }
        return;
    }
    if (auto array = std::get_if<Value::ArrayPtr>(&value.data)) {
        for (const auto& item : **array) {
            out << padding << "-";
            if (std::holds_alternative<Value::MapPtr>(item.data) || std::holds_alternative<Value::ArrayPtr>(item.data)) {
                out << "\n";
                yaml_dump_value(item, out, indent + 2);
            } else {
                out << " " << yaml_dump_scalar(item) << "\n";
            }
        }
        return;
    }
    out << padding << yaml_dump_scalar(value) << "\n";
}

} // namespace

std::vector<Value> format_arguments(const std::vector<Value>& args) {
    std::vector<Value> values;
    if (args.size() <= 1) return values;
    if (args.size() == 2 && std::holds_alternative<Value::ArrayPtr>(args[1].data)) {
        const auto& items = *std::get<Value::ArrayPtr>(args[1].data);
        values.assign(items.begin(), items.end());
        return values;
    }
    values.assign(args.begin() + 1, args.end());
    return values;
}

void install_extended_stdlib_builtins(BuiltinMap& builtins) {
    // ------------------------------ Digest ------------------------------
    builtins["__digest_digest"] = [](const std::vector<Value>& args) {
        require_count(args, 1, "Digest.digest");
        return digest_value(string_arg(args[0], "Digest.digest"), true, false);
    };
    builtins["__digest_hexdigest"] = [](const std::vector<Value>& args) {
        require_count(args, 1, "Digest.hexdigest");
        return digest_value(string_arg(args[0], "Digest.hexdigest"), false, false);
    };
    builtins["__digest_base64digest"] = [](const std::vector<Value>& args) {
        require_count(args, 1, "Digest.base64digest");
        return digest_value(string_arg(args[0], "Digest.base64digest"), false, true);
    };
    builtins["__digest_file"] = [](const std::vector<Value>& args) {
        require_count(args, 1, "Digest.file");
        return digest_value(read_binary(string_arg(args[0], "Digest.file")), false, false);
    };

    // ------------------------------ HMAC ------------------------------
    builtins["__openssl_hmac"] = [](const std::vector<Value>& args) {
        require_count(args, 2, "OpenSSL::HMAC");
        return Value(hex_encode(hmac_sha256(string_arg(args[0], "OpenSSL::HMAC"),
                                             string_arg(args[1], "OpenSSL::HMAC"))));
    };

    // ------------------------------ File / FileUtils ------------------------------
    builtins["__file_chmod"] = [](const std::vector<Value>& args) {
        require_count(args, 2, "File.chmod");
        const auto path = string_arg(args[0], "File.chmod");
        const auto mode = static_cast<unsigned>(integer_arg(args[1], "File.chmod"));
#ifdef _WIN32
        if (_chmod(path.c_str(), static_cast<int>(mode)) != 0)
            throw std::runtime_error("IOError: chmod failed: " + std::string(std::strerror(errno)));
#else
        if (::chmod(path.c_str(), static_cast<mode_t>(mode)) != 0)
            throw std::runtime_error("IOError: chmod failed: " + std::string(std::strerror(errno)));
#endif
        return Value(true);
    };
    builtins["__file_chown"] = [](const std::vector<Value>& args) -> Value {
        require_count(args, 3, "File.chown");
#ifdef _WIN32
        throw std::runtime_error("NotImplementedError: File.chown is not available on Windows");
#else
        if (::chown(string_arg(args[0], "File.chown").c_str(),
                    static_cast<uid_t>(integer_arg(args[1], "File.chown")),
                    static_cast<gid_t>(integer_arg(args[2], "File.chown"))) != 0)
            throw std::runtime_error("IOError: chown failed: " + std::string(std::strerror(errno)));
        return Value(true);
#endif
    };
    builtins["__fileutils_link"] = [](const std::vector<Value>& args) {
        require_count(args, 2, "FileUtils.ln");
        std::error_code ec;
        fs::create_hard_link(string_arg(args[0], "FileUtils.ln"), string_arg(args[1], "FileUtils.ln"), ec);
        if (ec) throw std::runtime_error("IOError: ln failed: " + ec.message());
        return Value(true);
    };
    builtins["__fileutils_symlink"] = [](const std::vector<Value>& args) {
        require_count(args, 2, "FileUtils.symlink");
        std::error_code ec;
        fs::create_symlink(string_arg(args[0], "FileUtils.symlink"), string_arg(args[1], "FileUtils.symlink"), ec);
        if (ec) throw std::runtime_error("IOError: symlink failed: " + ec.message());
        return Value(true);
    };

    // ------------------------------ Socket ------------------------------
    builtins["__socket_new"] = [](const std::vector<Value>& args) {
        require_range(args, 0, 1, "Socket.new");
        ensure_socket_runtime();
        SocketHandle handle = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (handle == invalid_socket) throw std::runtime_error("SocketError: " + socket_error());
        return socket_value(handle);
    };
    builtins["__socket_connect"] = [](const std::vector<Value>& args) {
        require_count(args, 3, "Socket.connect");
        ensure_socket_runtime();
        SocketHandle handle = socket_from_value(args[0], "Socket.connect");
        const std::string host = string_arg(args[1], "Socket.connect");
        const int port = static_cast<int>(integer_arg(args[2], "Socket.connect"));
        addrinfo hints{};
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        addrinfo* result = nullptr;
        const std::string service = std::to_string(port);
        if (getaddrinfo(host.c_str(), service.c_str(), &hints, &result) != 0 || !result)
            throw std::runtime_error("SocketError: cannot resolve host '" + host + "'");
        const int rc = ::connect(handle, result->ai_addr, static_cast<int>(result->ai_addrlen));
        freeaddrinfo(result);
        if (rc != 0) throw std::runtime_error("SocketError: connect failed: " + socket_error());
        return Value(true);
    };
    builtins["__socket_bind"] = [](const std::vector<Value>& args) {
        require_count(args, 3, "Socket.bind");
        ensure_socket_runtime();
        SocketHandle handle = socket_from_value(args[0], "Socket.bind");
        const std::string host = string_arg(args[1], "Socket.bind");
        const int port = static_cast<int>(integer_arg(args[2], "Socket.bind"));
        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_port = htons(static_cast<unsigned short>(port));
        if (host.empty() || host == "0.0.0.0") {
            address.sin_addr.s_addr = htonl(INADDR_ANY);
        } else {
#ifdef _WIN32
            const unsigned long raw_address = ::inet_addr(host.c_str());
            if (raw_address == INADDR_NONE && host != "255.255.255.255")
                throw std::runtime_error("SocketError: invalid bind address");
            address.sin_addr.s_addr = raw_address;
#else
            if (::inet_pton(AF_INET, host.c_str(), &address.sin_addr) != 1)
                throw std::runtime_error("SocketError: invalid bind address");
#endif
        }
        if (::bind(handle, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0)
            throw std::runtime_error("SocketError: bind failed: " + socket_error());
        return Value(true);
    };
    builtins["__socket_listen"] = [](const std::vector<Value>& args) {
        require_range(args, 1, 2, "Socket.listen");
        SocketHandle handle = socket_from_value(args[0], "Socket.listen");
        const int backlog = args.size() == 2 ? static_cast<int>(integer_arg(args[1], "Socket.listen")) : 16;
        if (::listen(handle, backlog) != 0)
            throw std::runtime_error("SocketError: listen failed: " + socket_error());
        return Value(true);
    };
    builtins["__socket_accept"] = [](const std::vector<Value>& args) {
        require_count(args, 1, "Socket.accept");
        SocketHandle handle = socket_from_value(args[0], "Socket.accept");
        sockaddr_in address{};
#ifdef _WIN32
        int length = sizeof(address);
#else
        socklen_t length = sizeof(address);
#endif
        SocketHandle client = ::accept(handle, reinterpret_cast<sockaddr*>(&address), &length);
        if (client == invalid_socket) throw std::runtime_error("SocketError: accept failed: " + socket_error());
        return socket_value(client);
    };
    builtins["__socket_recv"] = [](const std::vector<Value>& args) {
        require_range(args, 1, 2, "Socket.recv");
        SocketHandle handle = socket_from_value(args[0], "Socket.recv");
        const int size = args.size() == 2 ? static_cast<int>(integer_arg(args[1], "Socket.recv")) : 4096;
        if (size < 0 || size > 16 * 1024 * 1024) throw std::runtime_error("ValueError: invalid receive size");
        std::string buffer(static_cast<std::size_t>(size), '\0');
#ifdef _WIN32
        const int count = ::recv(handle, buffer.data(), size, 0);
#else
        const int count = static_cast<int>(::recv(handle, buffer.data(), static_cast<std::size_t>(size), 0));
#endif
        if (count < 0) throw std::runtime_error("SocketError: recv failed: " + socket_error());
        buffer.resize(static_cast<std::size_t>(count));
        return Value(buffer);
    };
    builtins["__socket_send"] = [](const std::vector<Value>& args) {
        require_count(args, 2, "Socket.send");
        SocketHandle handle = socket_from_value(args[0], "Socket.send");
        const std::string text = string_arg(args[1], "Socket.send");
#ifdef _WIN32
        const int sent = ::send(handle, text.data(), static_cast<int>(text.size()), 0);
#else
        const int sent = static_cast<int>(::send(handle, text.data(), text.size(), 0));
#endif
        if (sent < 0) throw std::runtime_error("SocketError: send failed: " + socket_error());
        return Value(static_cast<long long>(sent));
    };
    builtins["__socket_close"] = [](const std::vector<Value>& args) {
        require_count(args, 1, "Socket.close");
        auto object = map_arg(args[0], "Socket.close");
        SocketHandle handle = socket_from_value(args[0], "Socket.close");
        close_socket(handle);
        (*object)["closed"] = true;
        return Value(true);
    };

    // ------------------------------ Resolv ------------------------------
    builtins["__resolv_getaddress"] = [](const std::vector<Value>& args) {
        require_count(args, 1, "Resolv.getaddress");
        addrinfo hints{};
        hints.ai_family = AF_INET;
        addrinfo* result = nullptr;
        if (getaddrinfo(string_arg(args[0], "Resolv.getaddress").c_str(), nullptr, &hints, &result) != 0 || !result)
            throw std::runtime_error("ResolvError: address lookup failed");
        char buffer[INET_ADDRSTRLEN]{};
        auto* address = reinterpret_cast<sockaddr_in*>(result->ai_addr);
#ifdef _WIN32
        const char* text = ::inet_ntoa(address->sin_addr);
        if (text) std::strncpy(buffer, text, sizeof(buffer) - 1);
#else
        ::inet_ntop(AF_INET, &address->sin_addr, buffer, sizeof(buffer));
#endif
        freeaddrinfo(result);
        return Value(std::string(buffer));
    };
    builtins["__resolv_getname"] = [](const std::vector<Value>& args) {
        require_count(args, 1, "Resolv.getname");
        sockaddr_in address{};
        address.sin_family = AF_INET;
#ifdef _WIN32
        const unsigned long raw_address = ::inet_addr(string_arg(args[0], "Resolv.getname").c_str());
        if (raw_address == INADDR_NONE && string_arg(args[0], "Resolv.getname") != "255.255.255.255")
            throw std::runtime_error("ResolvError: invalid IPv4 address");
        address.sin_addr.s_addr = raw_address;
#else
        if (::inet_pton(AF_INET, string_arg(args[0], "Resolv.getname").c_str(), &address.sin_addr) != 1)
            throw std::runtime_error("ResolvError: invalid IPv4 address");
#endif
        char host[NI_MAXHOST]{};
        if (getnameinfo(reinterpret_cast<sockaddr*>(&address), sizeof(address), host, sizeof(host), nullptr, 0, 0) != 0)
            throw std::runtime_error("ResolvError: reverse lookup failed");
        return Value(std::string(host));
    };

    // ------------------------------ Time / Date ------------------------------
    builtins["__time_strptime"] = [](const std::vector<Value>& args) {
        require_count(args, 2, "Time.strptime");
        std::tm value{};
        std::istringstream input(string_arg(args[0], "Time.strptime"));
        input >> std::get_time(&value, string_arg(args[1], "Time.strptime").c_str());
        if (input.fail()) throw std::runtime_error("TimeParseError: invalid time");
        return Value(static_cast<long long>(std::mktime(&value)) * 1000);
    };
    builtins["__time_parts"] = [](const std::vector<Value>& args) {
        require_count(args, 1, "Time.parts");
        const auto millis = integer_arg(args[0], "Time.parts");
        const std::time_t raw = static_cast<std::time_t>(millis / 1000);
        std::tm local{};
#ifdef _WIN32
        localtime_s(&local, &raw);
#else
        localtime_r(&raw, &local);
#endif
        Map result;
        result["year"] = static_cast<long long>(local.tm_year + 1900);
        result["month"] = static_cast<long long>(local.tm_mon + 1);
        result["day"] = static_cast<long long>(local.tm_mday);
        result["hour"] = static_cast<long long>(local.tm_hour);
        result["minute"] = static_cast<long long>(local.tm_min);
        result["second"] = static_cast<long long>(local.tm_sec);
        result["weekday"] = static_cast<long long>(local.tm_wday);
        result["timestamp"] = millis;
        return Value(std::move(result));
    };
    builtins["__date_parse"] = [](const std::vector<Value>& args) {
        require_count(args, 1, "Date.parse");
        const std::string text = string_arg(args[0], "Date.parse");
        const char* formats[] = {"%Y-%m-%d", "%Y/%m/%d", "%d-%m-%Y", "%m/%d/%Y"};
        for (const char* format : formats) {
            std::tm value{};
            std::istringstream input(text);
            input >> std::get_time(&value, format);
            if (!input.fail()) return Value(static_cast<long long>(std::mktime(&value)) * 1000);
        }
        throw std::runtime_error("DateParseError: invalid date '" + text + "'");
    };
    builtins["__date_strptime"] = [](const std::vector<Value>& args) {
        require_count(args, 2, "Date.strptime");
        std::tm value{};
        std::istringstream input(string_arg(args[0], "Date.strptime"));
        input >> std::get_time(&value, string_arg(args[1], "Date.strptime").c_str());
        if (input.fail()) throw std::runtime_error("DateParseError: invalid date");
        return Value(static_cast<long long>(std::mktime(&value)) * 1000);
    };

    // ------------------------------ IO ------------------------------
    builtins["__io_open"] = [](const std::vector<Value>& args) {
        require_range(args, 1, 2, "IO.new");
        const std::string path = string_arg(args[0], "IO.new");
        const std::string mode = args.size() == 2 ? string_arg(args[1], "IO.new") : "r";
        if (mode.find('r') != std::string::npos && !fs::exists(path) && mode.find('w') == std::string::npos && mode.find('a') == std::string::npos)
            throw std::runtime_error("IOError: file does not exist: '" + path + "'");
        return make_file_descriptor(path, mode);
    };
    builtins["__io_seek"] = [](const std::vector<Value>& args) {
        require_range(args, 2, 3, "IO.seek");
        auto descriptor = descriptor_arg(args[0], "IO.seek");
        check_descriptor(descriptor, "IO.seek");
        const long long offset = integer_arg(args[1], "IO.seek");
        const long long whence = args.size() == 3 ? integer_arg(args[2], "IO.seek") : 0;
        long long base = 0;
        if (whence == 1) base = descriptor_position(descriptor);
        else if (whence == 2) base = static_cast<long long>(fs::file_size(string_arg(descriptor->at("path"), "IO.seek")));
        else if (whence != 0) throw std::runtime_error("ArgumentError: IO.seek whence must be 0, 1, or 2");
        const long long position = base + offset;
        if (position < 0) throw std::runtime_error("IOError: negative seek position");
        set_descriptor_position(descriptor, position);
        return Value(position);
    };
    builtins["__io_pos"] = [](const std::vector<Value>& args) {
        require_count(args, 1, "IO.pos");
        auto descriptor = descriptor_arg(args[0], "IO.pos");
        check_descriptor(descriptor, "IO.pos");
        return Value(descriptor_position(descriptor));
    };
    builtins["__io_rewind"] = [](const std::vector<Value>& args) {
        require_count(args, 1, "IO.rewind");
        auto descriptor = descriptor_arg(args[0], "IO.rewind");
        check_descriptor(descriptor, "IO.rewind");
        set_descriptor_position(descriptor, 0);
        return Value(0LL);
    };
    builtins["__io_eof"] = [](const std::vector<Value>& args) {
        require_count(args, 1, "IO.eof");
        auto descriptor = descriptor_arg(args[0], "IO.eof");
        check_descriptor(descriptor, "IO.eof");
        const auto path = string_arg(descriptor->at("path"), "IO.eof");
        return Value(descriptor_position(descriptor) >= static_cast<long long>(fs::file_size(path)));
    };
    builtins["__io_fileno"] = [](const std::vector<Value>& args) {
        require_count(args, 1, "IO.fileno");
        auto descriptor = descriptor_arg(args[0], "IO.fileno");
        check_descriptor(descriptor, "IO.fileno");
        return Value(descriptor_position(descriptor));
    };
    builtins["__io_read_file"] = [](const std::vector<Value>& args) {
        require_range(args, 1, 2, "IO.read");
        auto descriptor = descriptor_arg(args[0], "IO.read");
        check_descriptor(descriptor, "IO.read");
        const std::string path = string_arg(descriptor->at("path"), "IO.read");
        const std::string data = read_binary(path);
        const long long pos = descriptor_position(descriptor);
        if (args.size() == 1) {
            set_descriptor_position(descriptor, static_cast<long long>(data.size()));
            return Value(data.substr(static_cast<std::size_t>(pos)));
        }
        const long long count = integer_arg(args[1], "IO.read");
        if (count < 0) throw std::runtime_error("ArgumentError: IO.read size cannot be negative");
        const std::size_t start = static_cast<std::size_t>(std::min<long long>(pos, data.size()));
        const std::size_t amount = std::min<std::size_t>(static_cast<std::size_t>(count), data.size() - start);
        set_descriptor_position(descriptor, static_cast<long long>(start + amount));
        return Value(data.substr(start, amount));
    };
    builtins["__io_write_file"] = [](const std::vector<Value>& args) {
        require_count(args, 2, "IO.write");
        auto descriptor = descriptor_arg(args[0], "IO.write");
        check_descriptor(descriptor, "IO.write");
        const std::string path = string_arg(descriptor->at("path"), "IO.write");
        const std::string data = string_arg(args[1], "IO.write");
        const std::string mode = string_arg(descriptor->at("mode"), "IO.write");
        std::ios::openmode open_mode = std::ios::binary;
        if (mode.find('a') != std::string::npos) open_mode |= std::ios::app;
        else open_mode |= std::ios::in | std::ios::out;
        std::fstream file(path, open_mode);
        if (!file && mode.find('a') == std::string::npos) {
            file.clear();
            file.open(path, std::ios::binary | std::ios::out | std::ios::trunc);
        }
        if (!file) throw std::runtime_error("IOError: cannot open '" + path + "'");
        file.seekp(descriptor_position(descriptor));
        file.write(data.data(), static_cast<std::streamsize>(data.size()));
        if (!file) throw std::runtime_error("IOError: write failed");
        set_descriptor_position(descriptor, descriptor_position(descriptor) + static_cast<long long>(data.size()));
        return Value(static_cast<long long>(data.size()));
    };
    builtins["__io_close"] = [](const std::vector<Value>& args) {
        require_count(args, 1, "IO.close");
        auto descriptor = descriptor_arg(args[0], "IO.close");
        (*descriptor)["closed"] = true;
        return Value(true);
    };
    builtins["__io_pipe"] = [](const std::vector<Value>&) -> Value {
        throw std::runtime_error("NotImplementedError: IO.pipe requires native stream handles and is not exposed yet");
    };
    builtins["__io_popen"] = [](const std::vector<Value>& args) -> Value {
        require_count(args, 1, "IO.popen");
        const auto result = run_command(string_arg(args[0], "IO.popen"));
        Map value;
        value["output"] = result.output;
        value["status"] = static_cast<long long>(result.status);
        return Value(std::move(value));
    };

    // ------------------------------ Etc ------------------------------
    builtins["__etc_getlogin"] = [](const std::vector<Value>& args) {
        require_count(args, 0, "Etc.getlogin");
#ifdef _WIN32
        const char* user = std::getenv("USERNAME");
#else
        const char* user = std::getenv("USER");
#endif
        return user ? Value(std::string(user)) : Value{};
    };
    builtins["__etc_getpwnam"] = [](const std::vector<Value>& args) -> Value {
        require_count(args, 1, "Etc.getpwnam");
#ifdef _WIN32
        throw std::runtime_error("NotImplementedError: Etc.getpwnam is POSIX-specific");
#else
        passwd* entry = getpwnam(string_arg(args[0], "Etc.getpwnam").c_str());
        if (!entry) return Value{};
        Map result;
        result["name"] = std::string(entry->pw_name);
        result["uid"] = static_cast<long long>(entry->pw_uid);
        result["gid"] = static_cast<long long>(entry->pw_gid);
        result["home"] = std::string(entry->pw_dir ? entry->pw_dir : "");
        result["shell"] = std::string(entry->pw_shell ? entry->pw_shell : "");
        return Value(std::move(result));
#endif
    };
    builtins["__etc_getpwuid"] = [](const std::vector<Value>& args) -> Value {
        require_count(args, 1, "Etc.getpwuid");
#ifdef _WIN32
        throw std::runtime_error("NotImplementedError: Etc.getpwuid is POSIX-specific");
#else
        passwd* entry = getpwuid(static_cast<uid_t>(integer_arg(args[0], "Etc.getpwuid")));
        if (!entry) return Value{};
        Map result;
        result["name"] = std::string(entry->pw_name);
        result["uid"] = static_cast<long long>(entry->pw_uid);
        result["gid"] = static_cast<long long>(entry->pw_gid);
        result["home"] = std::string(entry->pw_dir ? entry->pw_dir : "");
        result["shell"] = std::string(entry->pw_shell ? entry->pw_shell : "");
        return Value(std::move(result));
#endif
    };

    // ------------------------------ Shellwords ------------------------------
    builtins["__shellwords_split"] = [](const std::vector<Value>& args) {
        require_count(args, 1, "Shellwords.split");
        Array result;
        for (const auto& item : shell_split(string_arg(args[0], "Shellwords.split"))) result.emplace_back(item);
        return Value(std::move(result));
    };
    builtins["__shellwords_escape"] = [](const std::vector<Value>& args) {
        require_count(args, 1, "Shellwords.escape");
        return Value(shell_escape(string_arg(args[0], "Shellwords.escape")));
    };
    builtins["__shellwords_join"] = [](const std::vector<Value>& args) {
        require_count(args, 1, "Shellwords.join");
        auto items = array_arg(args[0], "Shellwords.join");
        std::string result;
        for (std::size_t i = 0; i < items->size(); ++i) {
            if (i) result += ' ';
            result += shell_escape(items->at(i).to_string());
        }
        return Value(std::move(result));
    };

    // ------------------------------ StringScanner ------------------------------
    builtins["__stringscanner_new"] = [](const std::vector<Value>& args) {
        require_count(args, 1, "StringScanner.new");
        Map scanner;
        scanner["string"] = string_arg(args[0], "StringScanner.new");
        scanner["pos"] = 0LL;
        scanner["matched"] = Value{};
        scanner["pre_match"] = Value{};
        scanner["post_match"] = Value{};
        return Value(std::move(scanner));
    };
    builtins["__stringscanner_scan"] = [](const std::vector<Value>& args) {
        require_count(args, 2, "StringScanner.scan");
        auto scanner = map_arg(args[0], "StringScanner.scan");
        const std::string pattern = string_arg(args[1], "StringScanner.scan");
        const std::string text = string_arg(scanner->at("string"), "StringScanner.scan");
        const std::size_t pos = static_cast<std::size_t>(integer_arg(scanner->at("pos"), "StringScanner.scan"));
        std::regex regex(pattern);
        std::smatch match;
        const std::string remaining = text.substr(pos);
        if (!std::regex_search(remaining, match, regex, std::regex_constants::match_continuous)) return Value{};
        const std::string value = match.str();
        (*scanner)["matched"] = value;
        (*scanner)["pre_match"] = text.substr(0, pos);
        (*scanner)["post_match"] = text.substr(pos + value.size());
        (*scanner)["pos"] = static_cast<long long>(pos + value.size());
        return Value(value);
    };
    builtins["__stringscanner_scan_until"] = [](const std::vector<Value>& args) {
        require_count(args, 2, "StringScanner.scan_until");
        auto scanner = map_arg(args[0], "StringScanner.scan_until");
        const std::string pattern = string_arg(args[1], "StringScanner.scan_until");
        const std::string text = string_arg(scanner->at("string"), "StringScanner.scan_until");
        const std::size_t pos = static_cast<std::size_t>(integer_arg(scanner->at("pos"), "StringScanner.scan_until"));
        std::regex regex(pattern);
        std::smatch match;
        const std::string remaining = text.substr(pos);
        if (!std::regex_search(remaining, match, regex)) return Value{};
        const std::size_t end = pos + static_cast<std::size_t>(match.position() + match.length());
        const std::string value = text.substr(pos, end - pos);
        (*scanner)["matched"] = match.str();
        (*scanner)["pre_match"] = text.substr(0, pos);
        (*scanner)["post_match"] = text.substr(end);
        (*scanner)["pos"] = static_cast<long long>(end);
        return Value(value);
    };
    builtins["__stringscanner_check"] = [](const std::vector<Value>& args) -> Value {
        require_count(args, 2, "StringScanner.check");
        auto scanner = map_arg(args[0], "StringScanner.check");
        const std::string pattern = string_arg(args[1], "StringScanner.check");
        const std::string text = string_arg(scanner->at("string"), "StringScanner.check");
        const std::size_t pos = static_cast<std::size_t>(integer_arg(scanner->at("pos"), "StringScanner.check"));
        std::regex expression(pattern);
        std::smatch match;
        const std::string remaining = text.substr(pos);
        if (!std::regex_search(remaining, match, expression, std::regex_constants::match_continuous)) return Value{};
        return Value(match.str());
    };
    builtins["__stringscanner_check_until"] = [](const std::vector<Value>& args) -> Value {
        require_count(args, 2, "StringScanner.check_until");
        auto scanner = map_arg(args[0], "StringScanner.check_until");
        const std::string pattern = string_arg(args[1], "StringScanner.check_until");
        const std::string text = string_arg(scanner->at("string"), "StringScanner.check_until");
        const std::size_t pos = static_cast<std::size_t>(integer_arg(scanner->at("pos"), "StringScanner.check_until"));
        std::regex expression(pattern);
        std::smatch match;
        const std::string remaining = text.substr(pos);
        if (!std::regex_search(remaining, match, expression)) return Value{};
        const std::size_t end = pos + static_cast<std::size_t>(match.position() + match.length());
        return Value(text.substr(pos, end - pos));
    };
    builtins["__stringscanner_skip"] = [](const std::vector<Value>& args) -> Value {
        require_count(args, 2, "StringScanner.skip");
        auto scanner = map_arg(args[0], "StringScanner.skip");
        const std::string value = string_arg(args[1], "StringScanner.skip");
        const std::string text = string_arg(scanner->at("string"), "StringScanner.skip");
        const std::size_t pos = static_cast<std::size_t>(integer_arg(scanner->at("pos"), "StringScanner.skip"));
        if (text.compare(pos, value.size(), value) != 0) return Value{};
        (*scanner)["matched"] = value;
        (*scanner)["pre_match"] = text.substr(0, pos);
        (*scanner)["post_match"] = text.substr(pos + value.size());
        (*scanner)["pos"] = static_cast<long long>(pos + value.size());
        return Value(static_cast<long long>(value.size()));
    };

    // ------------------------------ Signals ------------------------------
    builtins["__signal_list"] = [](const std::vector<Value>& args) {
        require_count(args, 0, "Signal.list");
        Map result;
#ifdef SIGINT
        result["INT"] = static_cast<long long>(SIGINT);
#endif
#ifdef SIGTERM
        result["TERM"] = static_cast<long long>(SIGTERM);
#endif
#ifdef SIGQUIT
        result["QUIT"] = static_cast<long long>(SIGQUIT);
#endif
#ifdef SIGHUP
        result["HUP"] = static_cast<long long>(SIGHUP);
#endif
        return Value(std::move(result));
    };
    builtins["__signal_signame"] = [](const std::vector<Value>& args) {
        require_count(args, 1, "Signal.signame");
        const int signal = static_cast<int>(integer_arg(args[0], "Signal.signame"));
#ifdef SIGINT
        if (signal == SIGINT) return Value("INT");
#endif
#ifdef SIGTERM
        if (signal == SIGTERM) return Value("TERM");
#endif
#ifdef SIGQUIT
        if (signal == SIGQUIT) return Value("QUIT");
#endif
#ifdef SIGHUP
        if (signal == SIGHUP) return Value("HUP");
#endif
        return Value{};
    };
    builtins["__signal_trap"] = [](const std::vector<Value>& args) -> Value {
        require_count(args, 2, "Signal.trap");
        throw std::runtime_error("NotImplementedError: signal handlers require a native callback bridge");
    };

    // ------------------------------ Process ------------------------------
    builtins["__process_ppid"] = [](const std::vector<Value>& args) {
        require_count(args, 0, "Process.ppid");
#ifdef _WIN32
        return Value(0LL);
#else
        return Value(static_cast<long long>(getppid()));
#endif
    };
    builtins["__process_spawn"] = [](const std::vector<Value>& args) {
        require_count(args, 1, "Process.spawn");
        const auto result = run_command(string_arg(args[0], "Process.spawn"));
        return Value(static_cast<long long>(result.status));
    };
    builtins["__process_wait"] = [](const std::vector<Value>& args) {
        require_count(args, 1, "Process.wait");
        return Value(integer_arg(args[0], "Process.wait"));
    };
    builtins["__process_waitpid"] = [](const std::vector<Value>& args) {
        require_count(args, 1, "Process.waitpid");
        return Value(integer_arg(args[0], "Process.waitpid"));
    };
    builtins["__process_kill"] = [](const std::vector<Value>& args) -> Value {
        require_count(args, 2, "Process.kill");
#ifdef _WIN32
        throw std::runtime_error("NotImplementedError: Process.kill signal delivery is not implemented on Windows");
#else
        if (::kill(static_cast<pid_t>(integer_arg(args[1], "Process.kill")), static_cast<int>(integer_arg(args[0], "Process.kill"))) != 0)
            throw std::runtime_error("ProcessError: kill failed: " + std::string(std::strerror(errno)));
        return Value(true);
#endif
    };
    builtins["__process_uid"] = [](const std::vector<Value>& args) {
        require_count(args, 0, "Process.uid");
#ifdef _WIN32
        return Value(0LL);
#else
        return Value(static_cast<long long>(getuid()));
#endif
    };
    builtins["__process_gid"] = [](const std::vector<Value>& args) {
        require_count(args, 0, "Process.gid");
#ifdef _WIN32
        return Value(0LL);
#else
        return Value(static_cast<long long>(getgid()));
#endif
    };
    builtins["__process_euid"] = [](const std::vector<Value>& args) {
        require_count(args, 0, "Process.euid");
#ifdef _WIN32
        return Value(0LL);
#else
        return Value(static_cast<long long>(geteuid()));
#endif
    };
    builtins["__process_egid"] = [](const std::vector<Value>& args) {
        require_count(args, 0, "Process.egid");
#ifdef _WIN32
        return Value(0LL);
#else
        return Value(static_cast<long long>(getegid()));
#endif
    };
    builtins["__process_groups"] = [](const std::vector<Value>& args) {
        require_count(args, 0, "Process.groups");
        Array result;
#ifndef _WIN32
        const int count = getgroups(0, nullptr);
        if (count > 0) {
            std::vector<gid_t> groups(static_cast<std::size_t>(count));
            getgroups(count, groups.data());
            for (gid_t group : groups) result.emplace_back(static_cast<long long>(group));
        }
#endif
        return Value(std::move(result));
    };
    builtins["__process_clock_gettime"] = [](const std::vector<Value>& args) {
        require_range(args, 0, 1, "Process.clock_gettime");
        if (args.empty() || string_arg(args[0], "Process.clock_gettime") == "monotonic") {
            const auto now = std::chrono::steady_clock::now().time_since_epoch();
            return Value(static_cast<double>(std::chrono::duration<double>(now).count()));
        }
        if (string_arg(args[0], "Process.clock_gettime") == "realtime") {
            const auto now = std::chrono::system_clock::now().time_since_epoch();
            return Value(static_cast<double>(std::chrono::duration<double>(now).count()));
        }
        throw std::runtime_error("ArgumentError: unsupported clock");
    };

    // ------------------------------ Benchmark ------------------------------
    builtins["__benchmark_realtime"] = [](const std::vector<Value>& args) {
        require_count(args, 1, "Benchmark.realtime");
        const auto start = std::chrono::steady_clock::now();
        const std::string command = string_arg(args[0], "Benchmark.realtime");
        const auto result = run_command(command);
        (void)result;
        return Value(std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count());
    };

    // ------------------------------ YAML ------------------------------
    builtins["__yaml_load"] = [](const std::vector<Value>& args) {
        require_count(args, 1, "YAML.load");
        return yaml_load_simple(string_arg(args[0], "YAML.load"));
    };
    builtins["__yaml_dump"] = [](const std::vector<Value>& args) {
        require_count(args, 1, "YAML.dump");
        std::ostringstream out;
        yaml_dump_value(args[0], out, 0);
        return Value(out.str());
    };
    builtins["__yaml_load_file"] = [](const std::vector<Value>& args) {
        require_count(args, 1, "YAML.load_file");
        return yaml_load_simple(read_binary(string_arg(args[0], "YAML.load_file")));
    };

    // ------------------------------ Kernel ------------------------------
    builtins["__kernel_format"] = [](const std::vector<Value>& args) {
        if (args.empty()) throw std::runtime_error("ArgumentError: format expects at least one argument");
        const std::vector<Value> values = format_arguments(args);
        return Value(format_values(string_arg(args[0], "Kernel.format"), values));
    };
    builtins["__kernel_printf"] = [](const std::vector<Value>& args) {
        if (args.empty()) throw std::runtime_error("ArgumentError: printf expects at least one argument");
        const std::vector<Value> values = format_arguments(args);
        std::cout << format_values(string_arg(args[0], "Kernel.printf"), values);
        std::cout.flush();
        return Value{};
    };
    builtins["__kernel_inspect"] = [](const std::vector<Value>& args) {
        require_count(args, 1, "Kernel.inspect");
        return Value(args[0].to_string());
    };
    builtins["__kernel_to_a"] = [](const std::vector<Value>& args) {
        require_count(args, 1, "Kernel.to_a");
        if (std::holds_alternative<Value::ArrayPtr>(args[0].data)) return args[0];
        Array result{args[0]};
        return Value(std::move(result));
    };
    builtins["__kernel_to_h"] = [](const std::vector<Value>& args) {
        require_count(args, 1, "Kernel.to_h");
        if (std::holds_alternative<Value::MapPtr>(args[0].data)) return args[0];
        throw std::runtime_error("TypeError: value cannot be converted to a map");
    };
    builtins["__kernel_to_sym"] = [](const std::vector<Value>& args) {
        require_count(args, 1, "Kernel.to_sym");
        return Value(args[0].to_string());
    };
    builtins["__kernel_system"] = [](const std::vector<Value>& args) {
        require_count(args, 1, "Kernel.system");
        const auto result = run_command(string_arg(args[0], "Kernel.system"));
        if (!result.output.empty()) std::cout << result.output;
        return Value(result.status == 0);
    };
}

} // namespace lucy

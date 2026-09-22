#pragma once

#include "ast.hpp"

#include <exception>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace lucy {

struct ReturnSignal : std::exception {
    Value value;

    explicit ReturnSignal(Value v) : value(std::move(v)) {}
};

struct BreakSignal : std::exception {};
struct ContinueSignal : std::exception {};

class Environment : public std::enable_shared_from_this<Environment> {
public:
    struct Entry {
        Value value;
        bool constant;
    };

    explicit Environment(std::shared_ptr<Environment> parent = nullptr)
        : parent_(std::move(parent)) {}

    void define(const std::string& name, Value value, bool constant = false);
    void clear() { values_.clear(); }

    bool assign(const std::string& name, Value value);
    Value get(const std::string& name) const;
    bool local(const std::string& name) const;
    bool constant(const std::string& name) const;

    std::shared_ptr<Environment> parent() const { return parent_; }
    std::shared_ptr<Environment> root();

    const std::unordered_map<std::string, Entry>& values() const {
        return values_;
    }

private:
    std::unordered_map<std::string, Entry> values_;
    std::shared_ptr<Environment> parent_;
};

class Interpreter {
public:
    explicit Interpreter(std::vector<std::string> argv = {});
    ~Interpreter();
    Value run(const std::vector<StmtPtr>& statements, bool echo = false);
    Value execute(const StmtPtr& statement);
    Value evaluate(const ExprPtr& expression);

    void set_current_file(std::string path) {
        current_file_ = std::move(path);
    }

    void repl();

    // Returns completion candidates for the current REPL input.
    std::vector<std::string> completion_candidates(const std::string& input) const;

private:
    std::vector<std::pair<std::string, std::string>> list_modules() const;
    std::shared_ptr<Environment> globals_;
    std::shared_ptr<Environment> env_;
    std::vector<std::shared_ptr<Environment>> environments_;
    std::unordered_map<std::string, std::function<Value(const std::vector<Value>&)>> builtins_;
    std::string current_file_;
    std::vector<std::string> import_stack_;

    std::shared_ptr<Environment> make_environment(
        std::shared_ptr<Environment> parent);

    void install_builtins();

    Value call(
        const Value& value,
        const std::vector<Value>& arguments,
        const std::vector<std::string>& names = {});

    Value assign_target(
        const ExprPtr& target,
        const Token& operator_token,
        const Value& value);

    Value binary(
        const Value& left,
        const Token& operator_token,
        const Value& right);

    Value compound(
        const Value& left,
        const Token& operator_token,
        const Value& right);

    bool equal(const Value& left, const Value& right) const;
    std::string interpolate(const std::string& value);

    Value member_get(const Value& value, const std::string& name);
    void member_set(const Value& value, const std::string& name, Value new_value);

    void import_module(const ImportStmt& statement);
    std::string resolve_module(const std::string& module) const;
    std::string read_file(const std::string& path) const;
};


} // namespace lucy

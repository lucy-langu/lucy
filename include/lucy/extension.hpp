#pragma once

#include "value.hpp"

#include <functional>
#include <stdexcept>
#include <memory>
#include <string>
#include <vector>

namespace lucy {

class Interpreter;

using NativeFunction = std::function<Value(Interpreter &, const std::vector<Value> &)>;

Value make_native_function(Interpreter &interpreter, const std::string &name, NativeFunction fn);

struct NativeObject {
    virtual ~NativeObject() = default;

    // Return a callable/member value for a native object.
    // Throw std::runtime_error when the member does not exist.
    virtual Value get_member(Interpreter &, const std::string &) {
        throw std::runtime_error("NoMethodError: native object has no member");
    }

    virtual void set_member(Interpreter &, const std::string &, Value) {
        throw std::runtime_error("NoMethodError: native object is read-only");
    }

    virtual std::string name() const { return "native"; }
};

class ExtensionModuleBuilder {
public:
    ExtensionModuleBuilder() = default;
    ExtensionModuleBuilder(Interpreter *interpreter, std::string module)
        : interpreter_(interpreter), module_(std::move(module)) {}

    ExtensionModuleBuilder &function(const std::string &name, NativeFunction fn);
    ExtensionModuleBuilder &constant(const std::string &name, Value value);

private:
    Interpreter *interpreter_ = nullptr;
    std::string module_;
};

class ExtensionAPI {
public:
    static constexpr int API_VERSION = 1;

    ExtensionAPI(Interpreter &interpreter, std::string extension_name)
        : interpreter_(interpreter), extension_name_(std::move(extension_name)) {}

    int api_version() const { return API_VERSION; }
    const std::string &extension_name() const { return extension_name_; }

    ExtensionModuleBuilder module(const std::string &name);

private:
    Interpreter &interpreter_;
    std::string extension_name_;
};

// Every C++ extension must export this symbol.
using ExtensionInit = bool (*)(ExtensionAPI &);

#if defined(_WIN32)
#define LUCY_EXTENSION_EXPORT __declspec(dllexport)
#else
#define LUCY_EXTENSION_EXPORT __attribute__((visibility("default")))
#endif

#define LUCY_EXTENSION_INIT \
    extern "C" LUCY_EXTENSION_EXPORT bool lucy_extension_init(lucy::ExtensionAPI &api)

} // namespace lucy

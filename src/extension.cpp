#include "lucy/extension.hpp"
#include "lucy/runtime.hpp"

namespace lucy {

Value make_native_function(Interpreter &interpreter, const std::string &name, NativeFunction fn) {
    return interpreter.make_native_function(name, std::move(fn));
}

ExtensionModuleBuilder &ExtensionModuleBuilder::function(const std::string &name, NativeFunction fn) {
    if (!interpreter_)
        throw std::runtime_error("ExtensionError: invalid module builder");
    interpreter_->register_native_function(module_, name, std::move(fn));
    return *this;
}

ExtensionModuleBuilder &ExtensionModuleBuilder::constant(const std::string &name, Value value) {
    if (!interpreter_)
        throw std::runtime_error("ExtensionError: invalid module builder");
    interpreter_->register_native_constant(module_, name, std::move(value));
    return *this;
}

ExtensionModuleBuilder ExtensionAPI::module(const std::string &name) {
    if (name.empty())
        throw std::runtime_error("ExtensionError: module name cannot be empty");
    return ExtensionModuleBuilder(&interpreter_, name);
}

} // namespace lucy

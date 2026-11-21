#include <lucy/extension.hpp>

LUCY_EXTENSION_INIT {
    api.module("hello")
        .constant("version", "1.0")
        .function("greet", [](lucy::Interpreter &, const std::vector<lucy::Value> &args) {
            if (args.size() != 1)
                throw std::runtime_error("ArgumentError: hello.greet expects 1 argument");
            return lucy::Value("Hello, " + args[0].to_string());
        });
    return true;
}

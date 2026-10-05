#include <lucy/extension.hpp>
#include <stdexcept>

class Counter : public lucy::NativeObject {
public:
    explicit Counter(long long value) : value_(value) {}

    std::string name() const override { return "Counter"; }

    lucy::Value get_member(lucy::Interpreter &interpreter, const std::string &name) override {
        if (name == "value")
            return lucy::Value(value_);
        if (name == "increment") {
            return lucy::make_native_function(interpreter, "Counter.increment", [this](lucy::Interpreter &, const std::vector<lucy::Value> &args) {
                if (args.size() > 1)
                    throw std::runtime_error("ArgumentError: Counter.increment expects 0 or 1 arguments");
                value_ += args.empty() ? 1 : std::get<long long>(args[0].data);
                return lucy::Value(value_);
            });
        }
        throw std::runtime_error("NoMethodError: Counter has no member '" + name + "'");
    }

private:
    long long value_;
};

LUCY_EXTENSION_INIT {
    api.module("counter")
        .function("new", [](lucy::Interpreter &, const std::vector<lucy::Value> &args) {
            if (args.size() > 1)
                throw std::runtime_error("ArgumentError: counter.new expects 0 or 1 arguments");
            long long value = args.empty() ? 0 : std::get<long long>(args[0].data);
            return lucy::Value(std::static_pointer_cast<lucy::NativeObject>(std::make_shared<Counter>(value)));
        });
    return true;
}

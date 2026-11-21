#include "lucy/runtime.hpp"
#include "type_system.hpp"

#ifdef NDEBUG
#undef NDEBUG // Keep contract assertions active in Release builds.
#endif
#include <cassert>
#include <stdexcept>
#include <string>

int main()
{
    using namespace lucy;

    auto globals = std::make_shared<Environment>();
    globals->define("answer", Value(42), false, "Int");

    auto local = std::make_shared<Environment>(globals);
    assert(local->get("answer").to_string() == "42");
    assert(local->type_check("answer", Value(7)));
    assert(!local->type_check("answer", Value("wrong")));

    assert(local->assign("answer", Value(43)));
    assert(globals->get("answer").to_string() == "43");

    bool type_rejected = false;
    try {
        local->assign("answer", Value("not an integer"));
    } catch (const std::runtime_error &error) {
        type_rejected = std::string(error.what()).find("TypeError:") == 0;
    }
    assert(type_rejected);

    globals->define("fixed", Value("constant"), true);
    bool constant_rejected = false;
    try {
        local->assign("fixed", Value("changed"));
    } catch (const std::runtime_error &error) {
        constant_rejected = std::string(error.what()).find("NameError:") == 0;
    }
    assert(constant_rejected);

    assert(type_matches("Int", Value(1)));
    assert(type_matches("Number", Value(1.5)));
    assert(type_matches("String", Value("Lucy")));
    assert(type_matches("Any", Value{}));
    assert(!type_matches("Int", Value("1")));

    return 0;
}

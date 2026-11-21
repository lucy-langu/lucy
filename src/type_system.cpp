#include "type_system.hpp"
#include "lucy/ast.hpp"
#include <stdexcept>
namespace lucy {
bool type_matches(const std::string &expected, const Value &value)
{
    if (expected.empty() || expected == "Any" || expected == "any") return true;
    const std::string actual = value.type_name();
    if (expected == "Int" || expected == "int") return actual == "int";
    if (expected == "Double" || expected == "double" || expected == "Float" || expected == "float") return actual == "double";
    if (expected == "Number" || expected == "number") return value.is_number();
    if (expected == "String" || expected == "string") return actual == "string";
    if (expected == "Bool" || expected == "bool") return actual == "bool";
    if (expected == "Nil" || expected == "nil") return actual == "nil";
    if (expected == "Array" || expected == "array") return actual == "array";
    if (expected == "Map" || expected == "map") return actual == "map";
    if (expected == "Function" || expected == "function") return actual == "function";
    if (expected == "Class" || expected == "class") return actual == "class";
    if (expected == "Native" || expected == "native") return actual == "native";
    if (expected == "Instance" || expected == "instance") return actual == "instance";
    if (auto i = std::get_if<Value::InstancePtr>(&value.data)) return *i && (*i)->klass && (*i)->klass->name == expected;
    return false;
}

void require_type(const std::string &expected, const Value &value, const std::string &where)
{
    if (!type_matches(expected, value))
        throw std::runtime_error("TypeError: " + where + " expects " + expected + ", got " + value.type_name());
}


} // namespace lucy

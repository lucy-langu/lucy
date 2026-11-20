#include "lucy/lexer.hpp"
#include "lucy/parser.hpp"
#include "lucy/runtime.hpp"

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

using namespace lucy;

static bool contains(const std::vector<std::string>& values, const std::string& wanted) {
    return std::find(values.begin(), values.end(), wanted) != values.end();
}

static void run(Interpreter& interpreter, const std::string& source) {
    Lexer lexer(source);
    Parser parser(lexer.scan());
    interpreter.run(parser.parse());
}

int main() {
    Interpreter interpreter;
    run(interpreter, R"(
import text
let items = [1, 2, 3]
let name = "Lucy"
let config = {"port": 8080}
let flag = true
struct Point { x: Int, y: Int }
let point = Point.new(1, 2)
class String { func custom_greeting() { return "hello" } }
class Int { func custom_number() { return self } }
class Widget { func run() { return true } }
let widget = Widget.new()
func sample(x) { return x }
)");

    auto text_matches = interpreter.completion_candidates("text.");
    if (!contains(text_matches, "url_encode") || !contains(text_matches, "base64_encode")) return 1;

    auto string_matches = interpreter.completion_candidates("name.");
    if (!contains(string_matches, "upper") || !contains(string_matches, "split") || !contains(string_matches, "to_s") || !contains(string_matches, "inspect")) return 2;

    auto array_matches = interpreter.completion_candidates("items.");
    if (!contains(array_matches, "push") || !contains(array_matches, "map") || !contains(array_matches, "reduce") || !contains(array_matches, "sort")) return 3;

    auto instance_matches = interpreter.completion_candidates("point.");
    if (!contains(instance_matches, "x") || !contains(instance_matches, "y")) return 4;

    auto nested_matches = interpreter.completion_candidates("point.x.");
    if (!contains(nested_matches, "abs") || !contains(nested_matches, "to_int")) return 5;

    auto number_chain_matches = interpreter.completion_candidates("point.x.to");
    if (!contains(number_chain_matches, "to_int") || !contains(number_chain_matches, "to_string") || !contains(number_chain_matches, "to_s")) return 6;
    auto number_clamp_matches = interpreter.completion_candidates("point.x.cl");
    if (!contains(number_clamp_matches, "clamp")) return 16;

    auto bool_matches = interpreter.completion_candidates("flag.to_");
    if (!contains(bool_matches, "to_s")) return 15;
    auto bool_inspect_matches = interpreter.completion_candidates("flag.ins");
    if (!contains(bool_inspect_matches, "inspect")) return 17;

    auto map_matches = interpreter.completion_candidates("config.");
    if (!contains(map_matches, "fetch") || !contains(map_matches, "merge") || !contains(map_matches, "to_s")) return 14;

    auto function_matches = interpreter.completion_candidates("sample.ar");
    if (!contains(function_matches, "arity")) return 18;
    auto class_matches = interpreter.completion_candidates("Widget.su");
    if (!contains(class_matches, "superclass")) return 19;
    auto instance_api_matches = interpreter.completion_candidates("widget.fi");
    if (!contains(instance_api_matches, "fields")) return 20;

    auto prefix_matches = interpreter.completion_candidates("text.u");
    if (!contains(prefix_matches, "url_encode")) return 7;

    auto app_matches = interpreter.completion_candidates("app.");
    if (!contains(app_matches, "parser") || !contains(app_matches, "logger") || !contains(app_matches, "OptionParser")) return 8;

    auto time_matches = interpreter.completion_candidates("time.");
    if (!contains(time_matches, "now") || !contains(time_matches, "sleep")) return 9;

    auto number_matches = interpreter.completion_candidates("point.x.to_");
    if (!contains(number_matches, "to_binary") || !contains(number_matches, "to_hex") || !contains(number_matches, "to_octal")) return 10;

    auto extended_string = interpreter.completion_candidates("name.custom_");
    if (!contains(extended_string, "custom_greeting")) return 12;
    auto extended_number = interpreter.completion_candidates("point.x.custom_");
    if (!contains(extended_number, "custom_number")) return 13;

    auto globals = interpreter.completion_candidates("A");
    if (!contains(globals, "Array") || !contains(globals, "ARGV")) return 11;

    return 0;
}

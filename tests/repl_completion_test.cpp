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
struct Point { x: Int, y: Int }
let point = Point.new(1, 2)
)");

    auto text_matches = interpreter.completion_candidates("text.");
    if (!contains(text_matches, "url_encode") || !contains(text_matches, "base64_encode")) return 1;

    auto string_matches = interpreter.completion_candidates("name.");
    if (!contains(string_matches, "upper") || !contains(string_matches, "split")) return 2;

    auto array_matches = interpreter.completion_candidates("items.");
    if (!contains(array_matches, "push") || !contains(array_matches, "map")) return 3;

    auto instance_matches = interpreter.completion_candidates("point.");
    if (!contains(instance_matches, "x") || !contains(instance_matches, "y")) return 4;

    auto nested_matches = interpreter.completion_candidates("point.x.");
    if (!contains(nested_matches, "abs") || !contains(nested_matches, "to_int")) return 5;

    auto number_chain_matches = interpreter.completion_candidates("point.x.to");
    if (!contains(number_chain_matches, "to_int") || !contains(number_chain_matches, "to_string")) return 6;

    auto prefix_matches = interpreter.completion_candidates("text.u");
    if (!contains(prefix_matches, "url_encode")) return 7;

    return 0;
}

#pragma once

#include "value.hpp"

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace lucy {

using BuiltinMap = std::unordered_map<std::string, std::function<Value(const std::vector<Value>&)>>;

// Registers native primitives used by the extended Ruby-inspired standard library.
void install_extended_stdlib_builtins(BuiltinMap& builtins);

} // namespace lucy

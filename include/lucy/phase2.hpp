#pragma once

#include "value.hpp"
#include "extension.hpp"

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace lucy {

using BuiltinMap = std::unordered_map<std::string, std::function<Value(const std::vector<Value>&)>>;

// Registers the native primitives used by the Phase 2 standard library.
void install_phase2_builtins(BuiltinMap& builtins);

} // namespace lucy

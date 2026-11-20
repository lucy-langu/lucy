#pragma once
#include "lucy/value.hpp"
#include <string>
namespace lucy {
bool type_matches(const std::string &expected, const Value &value);
void require_type(const std::string &expected, const Value &value, const std::string &where);
} // namespace lucy

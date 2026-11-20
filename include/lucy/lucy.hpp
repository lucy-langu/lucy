#pragma once

#include <string>
#include <vector>

namespace lucy {

void run_file(
    const std::string& path,
    const std::vector<std::string>& argv = {});

void run_source(
    const std::string& source,
    const std::string& name = "<source>",
    const std::vector<std::string>& argv = {});

void repl();

} // namespace lucy

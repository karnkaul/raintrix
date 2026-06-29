#pragma once
#include "raintrix/defaults.hpp"

namespace raintrix {
auto generate_config(std::string const& path = defaults::config_path) -> bool;
void run_trix(std::string const& config_path) noexcept(false);
} // namespace raintrix

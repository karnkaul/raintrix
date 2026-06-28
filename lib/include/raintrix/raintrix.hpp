#pragma once
#include <string>

namespace raintrix {
inline auto const default_config_path = std::string{"raintrix.cfg"};

auto generate_config(std::string const& path = default_config_path) -> bool;
void run_trix(std::string const& config_path) noexcept(false);
} // namespace raintrix

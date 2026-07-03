#pragma once
#include <string>

namespace raintrix {
[[nodiscard]] auto generate_config() -> std::string;
void run_trix(std::string const& config_path) noexcept(false);
} // namespace raintrix

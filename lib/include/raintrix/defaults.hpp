#pragma once
#include <string>

namespace raintrix::defaults {
constexpr std::string_view char_set_v{"abcdefghijklmnopqrstuvwxyz0123456789"};
constexpr std::string_view resolution_v{"fullscreen"};
constexpr std::string_view trail_tint_v{"#00ff00ff"};
constexpr auto tile_height_v{32.0f};
constexpr auto max_trails_v{1000};
constexpr auto density_v{1.0f};
constexpr auto max_depth_v{4.0f};
constexpr auto speed_v{1.0f};

inline auto const config_path = std::string{"raintrix.conf"};
} // namespace raintrix::defaults

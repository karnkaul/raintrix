#pragma once
#include "cfg/variable.hpp"
#include "klib/string/c_string.hpp"
#include "kvf/color.hpp"
#include "raintrix/defaults.hpp"
#include <glm/vec2.hpp>
#include <string>
#include <variant>

namespace raintrix::detail {
struct Config {
	struct Fullscreen {};
	struct Windowed {
		glm::ivec2 size{};
	};

	struct Post;

	auto load_from(klib::CString path) -> bool;
	[[nodiscard]] auto to_string() const -> std::string;

	// Window.
	cfg::Variable<std::string> resolution{"RESOLUTION", std::string{defaults::resolution_v}};
	cfg::Variable<bool> keybind_exit_escape{"KEYBIND_EXIT_ESCAPE", true};
	cfg::Variable<bool> keybind_stats_f1{"KEYBIND_STATS_F1", false};
	cfg::Variable<std::string> vsync{"VSYNC"};

	// Trails.
	cfg::Variable<std::string> font_path{"FONT_PATH"};
	cfg::Variable<float> tile_height{"TILE_HEIGHT", defaults::tile_height_v};
	cfg::Variable<std::string> char_set{"CHAR_SET", std::string{defaults::char_set_v}};
	cfg::Variable<std::string> trail_tint{"TRAIL_TINT", std::string{defaults::trail_tint_v}};

	// Rain.
	cfg::Variable<int> max_trails{"MAX_TRAILS", defaults::max_trails_v};
	cfg::Variable<float> density{"DENSITY", defaults::density_v};
	cfg::Variable<float> max_depth{"MAX_DEPTH", defaults::max_depth_v};
	cfg::Variable<float> speed{"SPEED", defaults::speed_v};
};

struct Config::Post {
	[[nodiscard]] static auto process(Config& out) -> Post;

	std::variant<Fullscreen, Windowed> resolution{};
	kvf::Color trail_tint{};
};
} // namespace raintrix::detail

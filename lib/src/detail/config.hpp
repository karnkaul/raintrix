#pragma once
#include "cfg/variable.hpp"
#include "klib/string/c_string.hpp"
#include "raintrix/defaults.hpp"
#include <string>

namespace raintrix::detail {
struct Config {
	auto load_from(klib::CString path) -> bool;
	[[nodiscard]] auto save_to(klib::CString path) const -> bool;

	// Window.
	cfg::Variable<std::string> resolution{"RESOLUTION", std::string{defaults::resolution_v}};

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
} // namespace raintrix::detail

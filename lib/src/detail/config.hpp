#pragma once
#include "cfg/variable.hpp"
#include "klib/string/c_string.hpp"
#include <string>

namespace raintrix::detail {
struct Config {
	auto load_from(klib::CString path) -> bool;
	[[nodiscard]] auto save_to(klib::CString path) const -> bool;

	cfg::Variable<std::string> font_path{"FONT_PATH"};
	cfg::Variable<float> tile_height{"TILE_HEIGHT", 32.0f};
	cfg::Variable<std::string> char_set{"CHAR_SET", "abcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()"};
};
} // namespace raintrix::detail

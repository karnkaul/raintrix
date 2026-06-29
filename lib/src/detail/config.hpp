#pragma once
#include "cfg/variable.hpp"
#include "detail/char_set.hpp"
#include "klib/string/c_string.hpp"
#include <string>

namespace raintrix::detail {
struct Config {
	auto load_from(klib::CString path) -> bool;
	[[nodiscard]] auto save_to(klib::CString path) const -> bool;

	cfg::Variable<std::string> font_path{"FONT_PATH"};
	cfg::Variable<float> tile_height{"TILE_HEIGHT", 32.0f};
	cfg::Variable<std::string> char_set{"CHAR_SET", std::string{default_char_set_v}};
};
} // namespace raintrix::detail

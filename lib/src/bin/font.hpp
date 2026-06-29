#pragma once
#include <cstddef>
#include <span>

namespace raintrix::bin {
[[nodiscard]] auto font_bytes() -> std::span<std::byte const>;
} // namespace raintrix::bin

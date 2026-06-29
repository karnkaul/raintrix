#pragma once
#include <functional>

namespace raintrix {
template <typename Type, template <typename> typename LowerT, template <typename> typename UpperT>
struct Limit {
	[[nodiscard]] constexpr auto in_range(Type const value) const -> bool { return LowerT<Type>{}(value, lo) && UpperT<Type>{}(value, hi); }
	constexpr auto operator()(Type const value) const -> bool { return in_range(value); }

	Type lo{};
	Type hi{};
};

namespace limits {
inline constexpr auto tile_height_v = Limit<float, std::greater, std::less_equal>{.lo = 0.0f, .hi = 100.0f};
inline constexpr auto max_trails_v = Limit<int, std::greater, std::less_equal>{.lo = 0, .hi = 10000};
inline constexpr auto density_v = Limit<float, std::greater, std::less_equal>{.lo = 0.0f, .hi = 10.0f};
inline constexpr auto max_depth_v = Limit<float, std::greater_equal, std::less_equal>{.lo = 1.0f, .hi = 10.0f};
inline constexpr auto speed_v = Limit<float, std::greater_equal, std::less_equal>{.lo = 0.1f, .hi = 10.0f};
} // namespace limits
} // namespace raintrix

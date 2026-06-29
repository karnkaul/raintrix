#pragma once
#include "kvf/color.hpp"
#include "le2d/vertex.hpp"
#include <span>

namespace raintrix {
struct QuadView {
	[[nodiscard]] auto get_alpha() const -> float { return vertices.empty() ? 0.0f : vertices.front().color.w; }

	std::span<le::Vertex const> vertices{};
};

struct QuadViewMut {
	void set_color(kvf::Color const color) {
		for (auto& vertex : vertices) { vertex.color = color.to_linear(); }
	}

	void set_alpha(float const alpha) {
		for (auto& vertex : vertices) { vertex.color.w = alpha; }
	}

	[[nodiscard]] auto get_alpha() const -> float { return vertices.empty() ? 0.0f : vertices.front().color.w; }

	std::span<le::Vertex> vertices{};
};
} // namespace raintrix

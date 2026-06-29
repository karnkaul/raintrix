#pragma once
#include "detail/quad_view.hpp"
#include "klib/ptr.hpp"
#include "le2d/drawable/drawable.hpp"
#include "le2d/resource/font.hpp"
#include "le2d/transform.hpp"
#include "le2d/vertex_array.hpp"

namespace raintrix::detail {
class Column : public le::IDrawable {
  public:
	void draw(le::IRenderer& renderer) const final;

	void setup_tiles(le::IFontAtlas const& font_atlas, std::string_view text, float tile_height = 128.0f);

	[[nodiscard]] auto quad_count() const -> std::size_t;
	[[nodiscard]] auto quad_at(std::size_t index) -> QuadViewMut;
	[[nodiscard]] auto quad_at(std::size_t index) const -> QuadView;

	le::Transform transform{};
	kvf::Color tint{kvf::white_v};

  private:
	template <typename T, typename Self>
	[[nodiscard]] auto get_quad_at(this Self&& self, std::size_t index) -> T;

	klib::Ptr<le::ITexture const> m_atlas{};
	le::VertexArray m_tiles{};
};
} // namespace raintrix::detail

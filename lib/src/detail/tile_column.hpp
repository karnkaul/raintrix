#pragma once
#include "klib/ptr.hpp"
#include "le2d/drawable/drawable.hpp"
#include "le2d/resource/font.hpp"
#include "le2d/vertex_array.hpp"

namespace raintrix::detail {
class TileColumn : public le::IDrawable {
  public:
	void setup_tiles(le::IFontAtlas const& font_atlas, std::string_view text, float tile_height = 128.0f);

	void draw(le::IRenderer& renderer) const final;

	float x_offset{};

  private:
	le::VertexArray m_tiles{};
	klib::Ptr<le::ITexture const> m_atlas{};
};
} // namespace raintrix::detail

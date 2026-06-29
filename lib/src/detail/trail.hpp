#pragma once
#include "detail/column.hpp"
#include "kvf/time.hpp"
#include "le2d/random.hpp"
#include "raintrix/defaults.hpp"

namespace raintrix::detail {
struct TrailCreateInfo {
	std::string_view char_set{defaults::char_set_v}; // must be non-empty
	float tile_height{defaults::tile_height_v};		 // must be > 0 and <= 100
	kvf::Color trail_tint{kvf::green_v};			 // alpha (.w) must be 255
};

class Trail : public le::IDrawable {
  public:
	using CreateInfo = TrailCreateInfo;

	enum class Status : std::int8_t { Running, Completed };

	explicit Trail(gsl::not_null<le::Random*> random_engine, gsl::not_null<le::IFont*> font, CreateInfo const& create_info);

	void draw(le::IRenderer& renderer) const final { m_column.draw(renderer); }

	void tick(kvf::Seconds dt);

	void randomize_cells(int count);
	void start_fall(float speed, kvf::Seconds ttl);

	[[nodiscard]] auto get_status() const -> Status;

	le::Transform transform{};
	kvf::Seconds fade_rate{1s};

  private:
	void tick_head(kvf::Seconds dt);
	void tick_tail(kvf::Seconds dt);
	void deactivate();

	gsl::not_null<le::Random*> m_random;
	gsl::not_null<le::IFont*> m_font;
	CreateInfo m_info{};

	Column m_column{};
	std::string m_text_buffer{};

	kvf::Seconds m_advance_rate{};
	kvf::Seconds m_ttl{};

	kvf::Seconds m_advance_remain{};
	kvf::Seconds m_ttl_remain{};
	int m_head_index{};
	bool m_active{};
};
} // namespace raintrix::detail

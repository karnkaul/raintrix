#pragma once
#include "detail/trail.hpp"

namespace raintrix::detail {
struct RainCreateInfo {
	float world_width{500.0f};
	int cell_count{10};

	TrailCreateInfo trail_ci{};
	int max_trail_count{500};
	kvf::Seconds trail_spawn_rate{0.02s};
};

class Rain : public le::IDrawable {
  public:
	using CreateInfo = RainCreateInfo;

	explicit Rain(gsl::not_null<le::Random*> random_engine, gsl::not_null<le::IFont*> font, CreateInfo const& create_info);

	void draw(le::IRenderer& renderer) const final;

	void tick(kvf::Seconds dt);

  private:
	[[nodiscard]] auto get_fresh_trail() -> klib::Ptr<Trail>;

	void spawn_trail();

	gsl::not_null<le::Random*> m_random;
	gsl::not_null<le::IFont*> m_font;
	CreateInfo m_info{};

	std::vector<Trail> m_trails{};

	kvf::Seconds m_spawn_remain{};
};
} // namespace raintrix::detail

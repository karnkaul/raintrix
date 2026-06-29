#pragma once
#include "detail/trail.hpp"

namespace raintrix::detail {
struct RainCreateInfo {
	static constexpr auto max_trail_count_v{500};
	static constexpr auto trail_spawn_rate_v = kvf::Seconds{0.02s};
	static constexpr auto max_depth_v{4.0f};

	float world_width{500.0f};
	int cell_count{10};

	TrailCreateInfo trail_ci{};
	int max_trail_count{max_trail_count_v};			   // must be > 0
	kvf::Seconds trail_spawn_rate{trail_spawn_rate_v}; // must be > 0
	float max_depth{max_depth_v};					   // must be >= 1
	float speed{1.0f};								   // must be > 0.1 and < 10.0
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

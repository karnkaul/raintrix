#pragma once
#include "detail/trail.hpp"

namespace raintrix::detail {
struct RainCreateInfo {
	static constexpr auto max_trail_count_v{10000};
	static constexpr auto max_depth_v{4.0f};

	glm::vec2 world_size{500.0f};

	TrailCreateInfo trail_ci{};
	int max_trail_count{max_trail_count_v}; // must be > 0
	float density{1.0f};					// must be > 0 and < 10
	float max_depth{max_depth_v};			// must be >= 1
	float speed{1.0f};						// must be > 0.1 and < 10
};

class Rain : public le::IDrawable {
  public:
	using CreateInfo = RainCreateInfo;

	explicit Rain(gsl::not_null<le::Random*> random_engine, gsl::not_null<le::IFont*> font, CreateInfo const& create_info);

	void draw(le::IRenderer& renderer) const final;

	void tick(kvf::Seconds dt);

  private:
	[[nodiscard]] auto next_inactive_trail() -> klib::Ptr<Trail>;

	void spawn_trail();

	gsl::not_null<le::Random*> m_random;
	gsl::not_null<le::IFont*> m_font;
	CreateInfo m_info{};
	int m_cell_count{};
	kvf::Seconds m_spawn_rate{};
	kvf::Seconds m_base_ttl{};

	std::vector<Trail> m_trails{};

	kvf::Seconds m_spawn_remain{};
};
} // namespace raintrix::detail

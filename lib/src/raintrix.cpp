#include "raintrix/raintrix.hpp"
#include "bin/font.hpp"
#include "cfg/io.hpp"
#include "cfg/variable.hpp"
#include "detail/config.hpp"
#include "detail/rain.hpp"
#include "klib/file_io.hpp"
#include "klib/log/tagged.hpp"
#include "klib/visitor.hpp"
#include "kvf/util.hpp"
#include "le2d/context.hpp"
#include "le2d/input/action.hpp"
#include "le2d/input/action_mapping.hpp"
#include "le2d/input/router.hpp"
#include "le2d/random.hpp"
#include "le2d/shape/quad.hpp"
#include "raintrix/build_version.hpp"
#include "raintrix/limits.hpp"
#include "raintrix/panic.hpp"
#include <imgui.h>
#include <algorithm>
#include <array>
#include <fstream>
#include <optional>

namespace raintrix {

#pragma region common

namespace {
auto const log = klib::log::Tagged{"raintrix"};

auto to_lower(std::string_view const input) -> std::string {
	auto ret = std::string{};
	ret.reserve(input.size());
	for (char const c : input) { ret.push_back(char(std::tolower(static_cast<unsigned char>(c)))); }
	return ret;
}

auto is_whitespace(char const c) -> bool { return std::isspace(static_cast<unsigned char>(c)) != 0; }

auto trim_whitespace(std::string_view in) -> std::string_view {
	while (!in.empty() && is_whitespace(in.front())) { in.remove_prefix(1); }
	while (!in.empty() && is_whitespace(in.back())) { in.remove_suffix(1); }
	return in;
}
} // namespace

#pragma endregion

#pragma region cfg
namespace cfg {
namespace {
struct KeyValue {
	[[nodiscard]] static constexpr auto split(std::string_view const line) -> KeyValue {
		auto const i = line.find('=');
		if (i == std::string_view::npos) { return {}; }
		auto const key = trim_whitespace(line.substr(0, i));
		auto const value = trim_whitespace(line.substr(i + 1));
		return KeyValue{.key = key, .value = value};
	}

	std::string_view key{};
	std::string_view value{};
};
} // namespace

void Reader::track_variable(IVariable& variable) { m_variables.insert_or_assign(variable.get_key(), &variable); }

void Reader::parse_next(std::string_view line) {
	line = trim_whitespace(line);
	if (line.empty() || line.starts_with('#')) { return; }

	auto const key_value = KeyValue::split(line);
	if (key_value.key.empty()) { return; }

	auto const it = m_variables.find(key_value.key);
	if (it == m_variables.end()) { return; }

	it->second->parse(key_value.value);
}

void Reader::parse_stream(std::istream& is) {
	for (auto line = std::string{}; std::getline(is, line);) { parse_next(line); }
}

auto Reader::parse_file(klib::CString const path) -> bool {
	auto file = std::ifstream{path.c_str()};
	if (!file) { return false; }
	parse_stream(file);
	return true;
}

void Writer::write_header(std::string_view const text) {
	static constexpr auto hashes_v = std::string_view{"################\n"};
	if (!m_text.empty()) { m_text.push_back('\n'); }
	m_text.append(hashes_v);
	if (!text.empty()) { std::format_to(std::back_inserter(m_text), "### {}\n{}", text, hashes_v); }
}

void Writer::write_variable(IVariable const& variable, std::string_view const description, WriteStatus const status) {
	if (!m_text.empty()) { m_text.push_back('\n'); }
	if (!description.empty()) { std::format_to(std::back_inserter(m_text), "## {}\n", description); }
	if (status == WriteStatus::Commented) { m_text.append("# "); }
	std::format_to(std::back_inserter(m_text), "{} = {}\n", variable.get_key(), variable.serialize_value());
}

auto Writer::write_to(klib::CString const path) const -> bool {
	auto file = std::ofstream{path.c_str()};
	if (!file) { return false; }
	file.write(m_text.c_str(), std::streamsize(m_text.size()));
	return file.good();
}
} // namespace cfg

void cfg::assign_if(std::string_view input, bool& out) {
	auto const value = to_lower(input);
	auto const is_match = [&value](std::string_view const target) { return value == target; };

	static constexpr auto truthys = std::array{"true", "on", "y", "yes"};
	if (std::ranges::any_of(truthys, is_match)) {
		out = true;
		return;
	}

	static constexpr auto falseys = std::array{"false", "off", "no", "n"};
	if (std::ranges::any_of(falseys, is_match)) {
		out = false;
		return;
	}

	auto i = 0;
	if (assign_if(input, i)) { out = i != 0; }
}

void cfg::assign_if(std::string_view const input, std::string& out) { out = input; }

#pragma endregion

#pragma region config

namespace detail {
namespace {
template <typename T, typename FuncT>
auto check_valid(cfg::Variable<T> const& out, FuncT pred) -> bool {
	if (pred(out.value)) { return true; }
	log.warn("invalid {}: '{}'", out.get_key(), out.value);
	return false;
}

template <typename T, typename U, typename FuncT>
void ensure_valid(cfg::Variable<T>& out, U const& fallback, FuncT pred) {
	if (check_valid(out, pred)) { return; }
	out.value = fallback;
}

[[nodiscard]] auto to_windowed(std::string_view const input) -> std::optional<Config::Windowed> {
	auto const x = input.find('x');
	if (x == std::string_view::npos) { return {}; }
	auto const width_str = input.substr(0, x);
	auto const height_str = input.substr(x + 1);
	auto ret = glm::ivec2{};
	if (!cfg::assign_if(width_str, ret.x) || !cfg::assign_if(height_str, ret.y)) { return {}; }
	if (!kvf::is_positive(ret)) { return {}; }
	return Config::Windowed{.size = ret};
}
} // namespace

auto Config::load_from(klib::CString const path) -> bool {
	auto reader = cfg::Reader{};

	reader.track_variable(resolution);

	reader.track_variable(font_path);
	reader.track_variable(tile_height);
	reader.track_variable(char_set);
	reader.track_variable(trail_tint);

	reader.track_variable(max_trails);
	reader.track_variable(density);
	reader.track_variable(max_depth);
	reader.track_variable(speed);

	return reader.parse_file(path);
}

auto Config::save_to(klib::CString const path) const -> bool {
	auto writer = cfg::Writer{};

	writer.write_header("Window");
	writer.write_variable(resolution, "resolution (fullscreen|WxH)");

	writer.write_header("Trails");
	writer.write_variable(font_path, "path to custom font file");
	writer.write_variable(tile_height, "glyph tile height (== width) (0, 100]");
	writer.write_variable(char_set, "set to sample characters from (non-empty)");
	writer.write_variable(trail_tint, "tint (color) of trail in 4-channel hex form (#RRGGBBAA) (alpha must be ff)");

	writer.write_header("Rain");
	writer.write_variable(max_trails, "maximum number of trails (0, 10000]");
	writer.write_variable(density, "trail density (affects spawn rate) (0, 10]");
	writer.write_variable(max_depth, "trail max depth / z (affects average trail scale), [1, 10]");
	writer.write_variable(speed, "fall speed factor (affects average speed and time to live) (0.1, 10]");

	return writer.write_to(path);
}

auto Config::Post::process(Config& out) -> Config::Post {
	auto ret = Post{};
	check_valid(out.resolution, [&ret](std::string_view const in) {
		auto const resolution = to_lower(in);
		if (resolution == defaults::resolution_v) {
			ret.resolution = Fullscreen{};
			return true;
		}

		auto const windowed = to_windowed(resolution);
		if (!windowed || !kvf::is_positive(windowed->size)) { return false; }

		ret.resolution = *windowed;
		return true;
	});

	ensure_valid(out.tile_height, defaults::tile_height_v, limits::tile_height_v);
	ensure_valid(out.char_set, defaults::char_set_v, [](std::string_view const in) { return !in.empty(); });
	check_valid(out.trail_tint, [&ret](std::string_view const hex) {
		auto tint = kvf::util::color_from_hex(hex);
		if (tint.w != 0xff) { return false; }
		ret.trail_tint = tint;
		return true;
	});

	ensure_valid(out.max_trails, defaults::max_trails_v, limits::max_trails_v);
	ensure_valid(out.density, defaults::density_v, limits::density_v);
	ensure_valid(out.max_depth, defaults::max_depth_v, limits::max_depth_v);
	ensure_valid(out.speed, defaults::speed_v, limits::speed_v);

	return ret;
}
} // namespace detail

#pragma endregion

#pragma region column

namespace detail {
void Column::setup_tiles(le::IFontAtlas const& font_atlas, std::string_view const text, float const tile_height) {
	m_tiles.clear();
	if (text.empty()) { return; }

	auto glyph_layouts = std::vector<kvf::ttf::GlyphLayout>{};
	font_atlas.push_layouts(glyph_layouts, text);

	auto y_offset = 0.0f;

	m_tiles.reserve(le::shape::Quad::vertex_count_v * glyph_layouts.size(), le::shape::Quad::indices_v.size() * glyph_layouts.size());
	for (auto const& glyph_layout : glyph_layouts) {
		auto rect = kvf::Rect<>::from_size(glyph_layout.glyph->size, glm::vec2{0.0f, y_offset});
		auto quad = le::shape::Quad{};
		quad.create(rect, glyph_layout.glyph->uv_rect);
		m_tiles.append(quad.get_vertices(), quad.get_indices());
		y_offset -= tile_height;
	}

	m_atlas = &font_atlas.get_texture();
}

auto Column::quad_count() const -> std::size_t { return m_tiles.vertices.size() / le::shape::Quad::vertex_count_v; }

auto Column::quad_at(std::size_t const index) -> QuadViewMut { return get_quad_at<QuadViewMut>(index); }

auto Column::quad_at(std::size_t const index) const -> QuadView { return get_quad_at<QuadView>(index); }

void Column::draw(le::IRenderer& renderer) const {
	if (!m_atlas) { return; }

	auto const primitive = le::Primitive{
		.vertices = m_tiles.vertices,
		.indices = m_tiles.indices,
		.texture = m_atlas,
	};
	auto const render_instance = le::RenderInstance{
		.transform = transform,
		.tint = tint,
	};
	renderer.draw(primitive, {&render_instance, 1});
}

template <typename T, typename Self>
// NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward)
auto Column::get_quad_at(this Self&& self, std::size_t index) -> T {
	auto const vertex_index = index * le::shape::Quad::vertex_count_v;
	if (vertex_index + le::shape::Quad::vertex_count_v > self.m_tiles.vertices.size()) { return {}; }
	return T{.vertices = std::span{self.m_tiles.vertices}.subspan(vertex_index, le::shape::Quad::vertex_count_v)};
}
} // namespace detail

#pragma endregion

#pragma region trail

namespace detail {
Trail::Trail(gsl::not_null<le::Random*> random_engine, gsl::not_null<le::IFont*> font, CreateInfo const& create_info)
	: m_random(random_engine), m_font(font), m_info(create_info) {
	KLIB_ASSERT(limits::tile_height_v.in_range(m_info.tile_height));
	KLIB_ASSERT(m_info.trail_tint.w == 255);
}

void Trail::tick(kvf::Seconds const dt) {
	if (m_active) { tick_head(dt); }
	tick_tail(dt);
	if (m_active) {
		m_ttl_remain -= dt;
		if (m_ttl_remain <= 0s) { deactivate(); }
	}
	m_column.transform = transform;
}

void Trail::randomize_cells(int const count) {
	m_text_buffer.clear();
	m_text_buffer.reserve(std::size_t(count));
	for (int i = 0; i < count; ++i) {
		auto const index = m_random->next_index(m_info.char_set.size());
		auto const c = m_info.char_set.at(index);
		m_text_buffer.push_back(c);
	}

	auto const text_height = le::TextHeight(m_info.tile_height);
	m_column.setup_tiles(m_font->get_atlas(text_height), m_text_buffer, m_info.tile_height);
}

void Trail::start_fall(float const speed, kvf::Seconds const ttl) {
	m_advance_rate = kvf::Seconds{1.0f / speed};
	m_ttl = ttl;
	m_advance_remain = m_advance_rate;
	m_ttl_remain = m_ttl;
	m_head_index = 0;
	m_active = true;

	auto const quad_count = m_column.quad_count();
	for (auto i = 0uz; i < quad_count; ++i) {
		auto quad = m_column.quad_at(i);
		quad.set_color(m_info.trail_tint);
		quad.set_alpha(0.0f);
	}
	auto head = m_column.quad_at(0);
	head.set_color(kvf::white_v);
	head.set_alpha(1.0f);
}

auto Trail::get_status() const -> Status {
	if (m_active) { return Status::Running; }

	if (m_head_index == 0) { return Status::Completed; }
	auto const pre_head = m_column.quad_at(std::size_t(m_head_index - 1));
	if (pre_head.get_alpha() <= 0.0f) { return Status::Completed; }

	return Status::Running;
}

void Trail::tick_head(kvf::Seconds const dt) {
	m_advance_remain -= dt;
	if (m_advance_remain > 0s) { return; }

	m_advance_remain = m_advance_rate;

	auto const quad_count = int(m_column.quad_count());
	KLIB_ASSERT(m_head_index >= 0 && m_head_index < quad_count);
	auto head = m_column.quad_at(std::size_t(m_head_index));
	head.set_color(m_info.trail_tint);
	head.set_alpha(0.8f);

	++m_head_index;
	if (m_head_index >= quad_count) {
		head.set_color(m_info.trail_tint);
		m_active = false;
		return;
	}

	head = m_column.quad_at(std::size_t(m_head_index));
	head.set_color(kvf::white_v);
	head.set_alpha(1.0f);
}

void Trail::tick_tail(kvf::Seconds const dt) {
	if (m_head_index == 0) { return; }

	auto const d_alpha = dt / fade_rate;

	if (d_alpha <= 0.0f || d_alpha >= 1.0f) { return; }

	for (int index = m_head_index - 1; index >= 0; --index) {
		auto quad = m_column.quad_at(std::size_t(index));
		auto const alpha = std::max(0.0f, quad.get_alpha() - d_alpha);
		quad.set_alpha(alpha);
	}
}

void Trail::deactivate() {
	auto const quad_count = int(m_column.quad_count());
	KLIB_ASSERT(m_head_index >= 0 && m_head_index < quad_count);
	auto head = m_column.quad_at(std::size_t(m_head_index));
	head.set_alpha(0.0f);
	m_active = false;
}
} // namespace detail

#pragma endregion

#pragma region rain

namespace detail {
Rain::Rain(gsl::not_null<le::Random*> random_engine, gsl::not_null<le::IFont*> font, CreateInfo const& create_info)
	: m_random(random_engine), m_font(font), m_info(create_info) {
	KLIB_ASSERT(kvf::is_positive(m_info.world_size));
	KLIB_ASSERT(limits::max_trails_v.in_range(m_info.max_trail_count));
	KLIB_ASSERT(limits::max_depth_v.in_range(m_info.max_depth));
	KLIB_ASSERT(limits::speed_v.in_range(m_info.speed));
	KLIB_ASSERT(limits::density_v.in_range(m_info.density));

	m_spawn_rate = kvf::Seconds{60.0f / (m_info.density * m_info.world_size.x)};
	m_base_ttl = kvf::Seconds{m_info.world_size.y / 500.0f};
	m_base_fade_rate = kvf::Seconds{m_info.world_size.y / 500.0f};
	m_cell_count = (int(m_info.world_size.x) / int(m_info.trail_ci.tile_height)) + 1;
}

void Rain::draw(le::IRenderer& renderer) const {
	for (auto const& trail : m_trails) { trail.draw(renderer); }
}

void Rain::tick(kvf::Seconds const dt) {
	m_spawn_remain -= dt;
	if (m_spawn_remain <= 0s) {
		m_spawn_remain = m_spawn_rate;
		spawn_trail();
	}

	for (auto& trail : m_trails) { trail.tick(dt); }
}

auto Rain::next_inactive_trail() -> klib::Ptr<Trail> {
	for (auto& trail : m_trails) {
		if (trail.get_status() == Trail::Status::Completed) { return &trail; }
	}

	if (int(m_trails.size()) >= m_info.max_trail_count) { return nullptr; }

	auto& ret = m_trails.emplace_back(m_random, m_font, m_info.trail_ci);
	ret.randomize_cells(m_cell_count);
	return &ret;
}

void Rain::spawn_trail() {
	auto trail = next_inactive_trail();
	if (!trail) { return; }

	auto const half_width = 0.5f * m_info.world_size.x;
	trail->transform.position.x = m_random->next_float(-half_width, half_width);

	if (m_info.max_depth > 1.0f) {
		auto const min_scale = 1.0f / m_info.max_depth;
		trail->transform.scale = glm::vec2{m_random->next_float(min_scale, 1.0f)};
	}

	auto const speed = m_info.speed * m_random->next_float(10.0f, 20.0f);
	auto const ttl = m_base_ttl * (m_random->next_float(0.8f, 1.6f) / m_info.speed);
	trail->fade_rate = m_base_fade_rate * (m_random->next_float(0.8f, 1.6f));
	trail->start_fall(speed, ttl);
}
} // namespace detail

#pragma endregion

#pragma region app

namespace {
class App {
  public:
	void run(std::string const& config_path) {
		log.info("raintrix {}", build_version_v);

		load_config(config_path);
		load_font_bytes();
		create_context();
		create_font();
		create_rain();
		bind_actions();

		run_loop();
	}

  private:
	[[nodiscard]] auto get_window_ci() const -> le::WindowCreateInfo {
		auto const visitor = klib::Visitor{
			[&](detail::Config::Fullscreen) -> le::WindowCreateInfo { return le::FullscreenInfo{.title = "raintrix"}; },
			[&](detail::Config::Windowed const& w) -> le::WindowCreateInfo {
				auto ret = le::WindowInfo{.size = w.size, .title = "raintrix"};
				ret.flags &= ~le::WindowFlag::Visible;
				ret.flags &= ~le::WindowFlag::Decorated;
				return ret;
			},
		};
		return std::visit(visitor, m_post_config.resolution);
	}

	[[nodiscard]] auto get_trail_ci() const -> detail::Trail::CreateInfo {
		return detail::Trail::CreateInfo{
			.char_set = m_config.char_set.value,
			.tile_height = m_config.tile_height,
			.trail_tint = m_post_config.trail_tint,
		};
	}

	[[nodiscard]] auto get_rain_ci() const -> detail::Rain::CreateInfo {
		return detail::Rain::CreateInfo{
			.world_size = m_context->framebuffer_size(),
			.trail_ci = get_trail_ci(),
			.max_trail_count = m_config.max_trails,
			.density = m_config.density,
			.max_depth = m_config.max_depth,
			.speed = m_config.speed,
		};
	}

	void load_config(std::string const& path) {
		if (!path.empty()) {
			if (m_config.load_from(path)) {
				log.info("Config loaded from '{}'", path);
			} else {
				log.warn("failed to load Config from '{}'", path);
			}
		}
		m_post_config = detail::Config::Post::process(m_config);
	}

	void load_font_bytes() {
		if (!m_config.font_path.value.empty()) {
			auto const font_path = klib::CString{m_config.font_path.value};
			if (klib::read_file_bytes_into(m_font_bytes, font_path)) {
				log.info("font bytes loaded from '{}'", font_path);
			} else {
				log.warn("failed to load font from path: '{}'", font_path);
			}
		}

		if (m_font_bytes.empty()) {
			auto const font_bytes = bin::font_bytes();
			m_font_bytes = {font_bytes.begin(), font_bytes.end()};
		}
	}

	void create_context() {
		auto const context_ci = le::ContextCreateInfo{
			.window = get_window_ci(),
			.framebuffer_samples = vk::SampleCountFlagBits::e1,
		};
		m_context = le::Context::create(context_ci);
		if (!m_context) { throw Panic{"Failed to create Context"}; }

		m_waiter = m_context->create_waiter();

		ImGui::GetIO().IniFilename = nullptr;
	}

	void create_font() {
		auto const& resource_factory = m_context->get_resource_factory();
		m_font = resource_factory.create_font();
		if (!m_font->load_face(std::move(m_font_bytes))) { throw Panic{std::format("Failed to load font: '{}'", m_config.font_path.value)}; }
	}

	void create_rain() {
		auto const rain_ci = get_rain_ci();
		m_rain.emplace(&m_random_engine, m_font.get(), rain_ci);
	}

	void bind_actions() {
		m_action_mapping.bind_action(&m_exit, [this](le::input::action::Value const& v) {
			if (v.get<bool>()) { m_context->set_window_close(); }
		});

		m_input_router.push_mapping(&m_action_mapping);
	}

	void run_loop() {
		m_context->set_visible(true);

		while (m_context->is_running()) {
			m_context->next_frame();

			m_input_router.dispatch(m_context->event_queue());
			m_rain->tick(m_context->get_frame_stats().frame_dt);

			auto& renderer = m_context->begin_render();
			render(renderer);

			m_context->present();
		}
	}

	void render(le::IRenderer& renderer) const {
		renderer.view.position.y = -0.5f * float(renderer.framebuffer_size().y);
		m_rain->draw(renderer);
	}

	detail::Config m_config{};
	detail::Config::Post m_post_config{};

	std::unique_ptr<le::Context> m_context{};

	std::vector<std::byte> m_font_bytes{};
	std::unique_ptr<le::IFont> m_font{};

	le::Random m_random_engine{};

	le::input::Router m_input_router{};
	le::input::ActionMapping m_action_mapping{};
	le::input::action::KeyDigital m_exit{GLFW_KEY_ESCAPE};

	std::optional<detail::Rain> m_rain{};

	le::Context::Waiter m_waiter{};
};
} // namespace

#pragma endregion

} // namespace raintrix

auto raintrix::generate_config(std::string const& path) -> bool {
	auto const config = detail::Config{};
	return config.save_to(path);
}

void raintrix::run_trix(std::string const& config_path) { App{}.run(config_path); }

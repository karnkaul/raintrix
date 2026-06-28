#include "raintrix/raintrix.hpp"
#include "cfg/io.hpp"
#include "cfg/variable.hpp"
#include "detail/config.hpp"
#include "detail/tile_column.hpp"
#include "klib/file_io.hpp"
#include "klib/log/tagged.hpp"
#include "klib/random.hpp"
#include "le2d/context.hpp"
#include "le2d/shape/quad.hpp"
#include "raintrix/build_version.hpp"
#include "raintrix/panic.hpp"
#include <algorithm>
#include <array>
#include <fstream>

namespace raintrix {

#pragma region common

namespace {
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

void Writer::write_variable(IVariable const& variable, WriteStatus const status, std::string_view const description) {
	if (!m_text.empty()) { m_text.push_back('\n'); }
	if (!description.empty()) { std::format_to(std::back_inserter(m_text), "# {}\n", description); }
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

	static constexpr auto truthys = std::array{"true", "on"};
	if (std::ranges::any_of(truthys, is_match)) {
		out = true;
		return;
	}

	static constexpr auto falseys = std::array{"false", "off"};
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
[[nodiscard]] constexpr auto to_write_status(bool const active) { return active ? cfg::WriteStatus::Active : cfg::WriteStatus::Commented; }
} // namespace

auto Config::load_from(klib::CString const path) -> bool {
	auto reader = cfg::Reader{};

	reader.track_variable(font_path);
	reader.track_variable(tile_height);
	reader.track_variable(char_set);

	return reader.parse_file(path);
}

auto Config::save_to(klib::CString const path) const -> bool {
	auto writer = cfg::Writer{};

	writer.write_variable(font_path, to_write_status(!font_path.value.empty()), "path to font file");
	writer.write_variable(tile_height, cfg::WriteStatus::Active, "glyph tile height (== width)");
	writer.write_variable(char_set, cfg::WriteStatus::Active, "set to sample characters from");

	return writer.write_to(path);
}
} // namespace detail

#pragma endregion

#pragma region tile_column

namespace detail {
void TileColumn::setup_tiles(le::IFontAtlas const& font_atlas, std::string_view const text, float const tile_height) {
	m_tiles.clear();
	if (text.empty()) { return; }

	auto glyph_layouts = std::vector<kvf::ttf::GlyphLayout>{};
	font_atlas.push_layouts(glyph_layouts, text);

	auto const height = tile_height * float(text.size());
	auto y_offset = 0.5f * (height - tile_height);

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

void TileColumn::draw(le::IRenderer& renderer) const {
	if (!m_atlas) { return; }

	auto const primitive = le::Primitive{
		.vertices = m_tiles.vertices,
		.indices = m_tiles.indices,
		.texture = m_atlas,
	};
	auto render_instance = le::RenderInstance{};
	render_instance.transform.position.x = x_offset;
	renderer.draw(primitive, {&render_instance, 1});
}

} // namespace detail

#pragma endregion

#pragma region app

namespace {
class App {
  public:
	void run(std::string const& config_path) {
		m_log.info("raintrix {}", build_version_v);

		load_config(config_path);
		load_font_bytes();
		create_context();
		create_font();
		create_columns();

		run_loop();
	}

  private:
	void load_config(std::string const& path) {
		if (path.empty()) { return; }

		if (m_config.load_from(path)) {
			m_log.info("Config loaded from '{}'", path);
		} else {
			m_log.warn("failed to load Config from '{}'", path);
		}
	}

	void load_font_bytes() {
		auto const font_path = klib::CString{m_config.font_path.value};
		if (!klib::read_file_bytes_into(m_font_bytes, font_path)) { throw Panic{std::format("Failed to load font: '{}'", font_path)}; }
		m_log.info("font bytes loaded from '{}'", font_path);
	}

	void create_context() {
		auto window_ci = le::WindowInfo{.size = {1280, 720}, .title = "raintrix"};
		window_ci.flags &= ~le::WindowFlag::Visible;

		auto const context_ci = le::ContextCreateInfo{
			.window = le::WindowInfo{.size = {1280, 720}, .title = "raintrix"},
			.framebuffer_samples = vk::SampleCountFlagBits::e1,
		};
		m_context = le::Context::create(context_ci);
		if (!m_context) { throw Panic{"Failed to create Context"}; }

		m_waiter = m_context->create_waiter();
	}

	void create_font() {
		auto const& resource_factory = m_context->get_resource_factory();
		m_font = resource_factory.create_font();
		if (!m_font->load_face(std::move(m_font_bytes))) { throw Panic{std::format("Failed to load font: '{}'", m_config.font_path.value)}; }
	}

	void create_columns() {
		auto const fb_extent = m_context->get_render_device().get_framebuffer_extent();
		glm::vec2 const fb_size = glm::ivec2{fb_extent.width, fb_extent.height};

		auto const row_count = (int(fb_size.y) / int(m_config.tile_height.value)) + 1;
		auto const column_count = (int(fb_size.x) / int(m_config.tile_height.value)) + 1;

		auto x_offset = -0.5f * (fb_size.x + m_config.tile_height);
		for (int i = 0; i < column_count; ++i) {
			m_columns.push_back(create_column(row_count, x_offset));
			x_offset += m_config.tile_height;
		}
	}

	[[nodiscard]] auto create_column(int const char_count, float const x_offset) -> detail::TileColumn {
		auto ret = detail::TileColumn{};

		auto text = std::string{};
		text.reserve(std::size_t(char_count));
		for (int i = 0; i < char_count; ++i) {
			auto const index = klib::random_index(m_random_engine, m_config.char_set.value.size());
			auto const c = m_config.char_set.value.at(index);
			text.push_back(c);
		}

		auto const text_height = le::TextHeight(m_config.tile_height.value);
		ret.setup_tiles(m_font->get_atlas(text_height), text, m_config.tile_height);
		ret.x_offset = x_offset;

		return ret;
	}

	void run_loop() {
		while (m_context->is_running()) {
			m_context->next_frame();

			tick(m_context->get_frame_stats().frame_dt);

			auto& renderer = m_context->begin_render();
			render(renderer);

			m_context->present();
		}
	}

	void tick(kvf::Seconds const dt) {}

	void render(le::IRenderer& renderer) const {
		for (auto const& column : m_columns) { column.draw(renderer); }
	}

	klib::log::Tagged m_log{"raintrix"};

	detail::Config m_config{};

	std::unique_ptr<le::Context> m_context{};

	std::vector<std::byte> m_font_bytes{};
	std::unique_ptr<le::IFont> m_font{};

	std::default_random_engine m_random_engine{std::random_device{}()};

	std::vector<detail::TileColumn> m_columns{};

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

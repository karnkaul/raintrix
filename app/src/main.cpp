#include "clap/parser.hpp"
#include "raintrix/build_version.hpp"
#include "raintrix/raintrix.hpp"
#include <print>

namespace {
class App {
  public:
	auto run(int const argc, char const* const* argv) -> int {
		auto const parse_result = parse_args(argc, argv);
		if (parse_result.should_early_exit()) { return parse_result.return_code(); }

		if (m_generate_config) {
			std::println("{}", raintrix::generate_config());
			return EXIT_SUCCESS;
		}

		raintrix::run_trix(m_config_path);
		return EXIT_SUCCESS;
	}

  private:
	[[nodiscard]] auto parse_args(int const argc, char const* const* argv) -> clap::Result {
		auto spec = clap::spec::Parameters{
			.parameters =
				{
					clap::named_flag(m_generate_config, "g,generate", "generate config"),
					clap::positional_optional(m_config_path, "CONFIG", "path to config"),
				},
			.program =
				{
					.name = "raintrix",
					.version = raintrix::build_version_v,
				},
		};
		auto parser = clap::Parser{std::move(spec)};
		return parser.parse_main(argc, argv);
	}

	std::string m_config_path{};
	bool m_generate_config{};
};
} // namespace

int main(int argc, char** argv) {
	try {
		return App{}.run(argc, argv);
	} catch (std::exception const& e) {
		std::println(stderr, "PANIC: {}", e.what());
		return EXIT_FAILURE;
	} catch (...) {
		std::println("PANIC!");
		return EXIT_FAILURE;
	}
}

#include "clap/parser.hpp"
#include "raintrix/raintrix.hpp"
#include <print>

namespace {
auto run(int const argc, char const* const* argv) -> int {
	auto config_path = raintrix::default_config_path;
	auto generate_config = false;
	auto spec = clap::spec::Parameters{
		.parameters =
			{
				clap::named_option(config_path, "c,config", "path to config"),
				clap::named_flag(generate_config, "g,generate", "generate config"),
			},
		.program =
			{
				.name = "raintrix",
				// TODO: version
			},
	};
	auto parser = clap::Parser{std::move(spec)};
	auto const parse_result = parser.parse_main(argc, argv);
	if (parse_result.should_early_exit()) { return parse_result.return_code(); }

	if (generate_config) {
		auto const result = raintrix::generate_config(config_path);
		if (!result) {
			std::println(stderr, "Failed to store generated config to '{}'", config_path);
			return EXIT_FAILURE;
		}
		std::println("Config generated to '{}'", config_path);
		return EXIT_SUCCESS;
	}

	raintrix::run_trix(config_path);
	return EXIT_SUCCESS;
}
} // namespace

int main(int argc, char** argv) {
	try {
		return run(argc, argv);
	} catch (std::exception const& e) {
		std::println(stderr, "PANIC: {}", e.what());
		return EXIT_FAILURE;
	} catch (...) {
		std::println("PANIC!");
		return EXIT_FAILURE;
	}
}

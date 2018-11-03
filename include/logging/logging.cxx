#include <iostream>
#include <logging.hxx>

auto parsec::logging::get(const char* name) -> std::shared_ptr<spdlog::logger>
{
	if (const auto console = spdlog::get(name); console != nullptr) {
		return console;
	}
#ifndef NDEBUG
	const auto console = spdlog::stdout_color_st(name);
	// const auto console = spdlog::create<spdlog::stdout_color_st>(name);
	console->set_level(spdlog::level::trace);

#else
	const auto console = spdlog::create<spdlog::sinks::null_sink_st>(name);

#endif

	return console;
}

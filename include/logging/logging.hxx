#pragma once

#include <memory>

#include <spdlog/spdlog.h>

// colourful output for debug messages
#ifndef NDEBUG
#include <spdlog/sinks/stdout_color_sinks.h>

// discard all logged messages
#else
#include <spdlog/sinks/null_sink.h>

#endif

namespace parsec::logging {

using logger = std::shared_ptr<spdlog::logger>;

auto get(const char*) -> logger;

template <typename... Args>
auto debug(const logger& l, const char* fmt, Args&&... args) -> void
{
#ifndef NDEBUG
	l->debug(fmt, std::move(args)...);
#endif
}

template <typename... Args>
auto info(const logger& l, const char* fmt, Args&&... args) -> void
{
#ifndef NDEBUG
	l->info(fmt, std::move(args)...);
#endif
}

} // namespace parsec::logging
#pragma once

#include <logging/logging.hxx>

namespace parsec::logging {

class loggable {
  public:
	loggable() = delete;

  protected:
	loggable(const char* name) : console{get(name)}
	{
	}

	logger console;
};

} // namespace parsec::logging
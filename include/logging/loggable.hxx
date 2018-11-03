#pragma once

#include <logging/logging.hxx>

namespace parsec::logging {

class loggable {
  public:
	loggable() = delete;

    logger console;

protected:
	loggable(const char* name) : console{get(name)}
	{
	}

};

} // namespace parsec::logging
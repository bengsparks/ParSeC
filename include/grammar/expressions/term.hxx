#pragma once

#include <string>

#include <grammar/expressions/base_expr.hxx>

#include <logging/loggable.hxx>
#include <logging/logging.hxx>

namespace parsec::expr {

class term : public base_expr, public parsec::logging::loggable {
  public:
	/*
	 * Construct an instance of this class by passing a simple string
	 */
	explicit term(std::string);

	auto evaluate(std::string& input) const
		-> const std::unique_ptr<ast::base_node> override;

  private:
	std::string __m_terminal;
};

} // namespace parsec::expr

#pragma once

#include <memory>
#include <string>

#include <ast/base_node.hxx>

namespace parsec::expr {

class base_expr {
  public:
	virtual ~base_expr() = default;

	/*
	 * All class that derive from parsec::base_expr must have the ability to
	 * match a string. Should the string be matched correctly, evaluate MUST
	 * call parsec::base_expr::set_consumed
	 */
	virtual auto evaluate(std::string &) const
		-> const std::unique_ptr<ast::base_node> = 0;

	/*
	 * Returns the string that was set in parsec::base_expr::evaluate.
	 * This method must be declared virtual, in order for classes such as
	 * parsec::seq
	 */
	virtual auto consumed() const -> const std::string;

    virtual auto set_consumed(std::string) const -> void;

  protected:
	base_expr() = default;

	mutable std::string __m_consumed;
};

} // namespace parsec

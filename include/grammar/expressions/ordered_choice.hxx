#pragma once

#include <tuple>

#include <ast/node_from_expr.hxx>
#include <ast/ordered_choice.hxx>
#include <grammar/exceptions/ordered_choice.hxx>
#include <grammar/expressions/base_expr.hxx>
#include <logging/logging.hxx>

namespace parsec::expr {

template <typename... Exprs>
class ordered_choice : public base_expr, public parsec::logging::loggable {
  public:
	static_assert((std::is_base_of_v<base_expr, Exprs> && ...),
				  "All provided expressions must be derivatives of parsec::expr");

	explicit ordered_choice(Exprs&&... exprs) :
		// Initialise the logger
		parsec::logging::loggable{"parsec::expr::ordered_choice"},
		// Copy the expressions
		__m_expressions{std::make_tuple<Exprs...>(std::move(exprs)...)}
	{
	}

	auto evaluate(std::string& input) const
		-> const std::unique_ptr<ast::base_node> override
	{
		// Copy the input to avoid accidentally consuming the input when the
		// matching fails
		std::string __input{input};

		// Store the consumed section of the input here
		std::ostringstream consumed;

		evaluate(__input, consumed, std::index_sequence_for<Exprs...>{});

		// successfully matched, consume input
		set_consumed(consumed.str());
		input = __input;

		return std::make_unique<
			parsec::ast::ordered_choice<parsec::ast::node_from_expr_t<Exprs>...>>(*this);
	}

	template <typename Expr1, typename Expr2>
	friend auto operator|(Expr1, Expr2) -> ordered_choice<Expr1, Expr2>;

	template <typename... PreviousExprs, typename OtherExpr>
	friend auto operator|(ordered_choice<PreviousExprs...>, OtherExpr)
		-> ordered_choice<PreviousExprs..., OtherExpr>;

  private:
	template <std::size_t... Is>
	auto evaluate(std::string& input,
				  std::ostringstream& consumed,
				  std::index_sequence<Is...>) const -> void
	{
		// Accumulate errors here, should the evaluating fail entirely
		std::ostringstream errs;

		// Because true is returned by evaluate as soon as the first expression
		// matches the input, the short-circuit characteristic of the ||
		// operator is triggered, preventing further evaluation
		bool success = (evaluate<Is>(input, errs, consumed) || ...);
		if (not success) {
			throw exception::ordered_choice{"While attempting to match '" + input +
											"', the following exceptions were raised:\n" +
											errs.str()};
		}
	}

	/*
	 * Attempt to match the Ith expression with the input
	 */
	template <std::size_t I>
	auto evaluate(std::string& input,
				  std::ostringstream& errs,
				  std::ostringstream& consumed) const -> bool
	{
		try {
			logging::debug(console,
						   "calling std::get<{}>(__m_expressions).evaluate('{}')", I,
						   input);
			// attempt successfully evaluation
			std::get<I>(__m_expressions).evaluate(input);
		}

		// evaluation failed, log error, and indicate failure
		catch (const exception::base_exception& be) {
			logging::debug(console, "{}", be.what());
			errs << be.what() << '\n';
			return false;
		}

		// evaluation successful
		logging::info(console, "match found with std::get<{}>(__m_expressions)", I);

		logging::debug(console, "consumed '{}'", std::get<I>(__m_expressions).consumed());

		//  append the result of the evaluation to the stream
		consumed << std::get<I>(__m_expressions).consumed();

		// Return true to trigger the short-circuit behaviour of the || operator,
		// thereby indicating success
		return true;
	}

	std::tuple<Exprs...> __m_expressions;
};
/*
 * Template deduction guide intended to guarantee compilation across standard
 * conforming compilers
 */
template <typename Expr1, typename Expr2>
ordered_choice(Expr1, Expr2)->ordered_choice<Expr1, Expr2>;

/*
 * Generate parsec::expr::ordered_choice by specifying a series of expressions
 * separated by |
 */
template <typename Expr1, typename Expr2>
auto operator|(Expr1 expr1, Expr2 expr2) -> ordered_choice<Expr1, Expr2>
{
	return ordered_choice{std::move(expr1), std::move(expr2)};
}

/*
 * Template deduction guide intended to guarantee compilation across standard
 * conforming compilers
 */
template <typename... PreviousExprs, typename OtherExpr>
ordered_choice(ordered_choice<PreviousExprs...>, OtherExpr)
	->ordered_choice<PreviousExprs..., OtherExpr>;

/*
 * To avoid instantiation of chained instances of expr::ordered_choice (for example
 * expr::ordered_choice<expr::term, expr::ordered_choice<expr::term, expr::term>>), we
 * generate flattened instances of expr::ordered_choice by appending any expression
 * that follows expr::ordered_choice to that same instance of expr::ordered_choice, so
 * that the result reads expr::ordered_choice<expr::term, expr::term, expr::term> instead
 *
 * If this is not desired behaviour, explicitly call the constructor instead of
 * relying on the overloaded comma operator
 */
template <typename... PreviousExprs, typename OtherExpr>
auto operator|(ordered_choice<PreviousExprs...> opt, OtherExpr other)
	-> ordered_choice<PreviousExprs..., OtherExpr>
{
	// There is no need to check here whether PreviousExprs or OtherExpr is
	// actually made up of purely expressions, because expr::ordered_choice itself
	// checks for this
	return ordered_choice{std::move(opt), std::move(other)};
}
} // namespace parsec::expr

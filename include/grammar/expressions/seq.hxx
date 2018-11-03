#pragma once

#include <memory>
#include <numeric>
#include <sstream>
#include <utility>

#include <ast/node_from_expr.hxx>
#include <ast/seq.hxx>
#include <grammar/exceptions/seq.hxx>
#include <grammar/expressions/base_expr.hxx>

#include <logging.hxx>

namespace parsec::expr {

template <typename... Exprs>
class seq : public base_expr, public parsec::logging::loggable {
  public:
    using expr_tuple = std::tuple<Exprs...>;

	static_assert(
		(std::is_base_of_v<base_expr, Exprs> && ...),
		"All provided expressions must be derivatives of parsec::expr");

	/*
	 * Constructor for seq accepts any number of expressions
	 * that are to be matched sequentially
	 */
	explicit seq(Exprs&&... expressions) :
		// Initialise every expression
		parsec::logging::loggable{"parsec::expr::seq"},
		__m_expressions{std::make_tuple<Exprs...>(std::move(expressions)...)}
	{
	}

	/*
	 * Constructor for seq accepts an already constructed seq, as well as a
	 * second expression, that is to be added to the series of expressions to be
	 * matched sequentially
	 */
	template <typename... PreviousExprs, typename Expr>
	seq(seq<PreviousExprs...> sequence, Expr expr) :
		// Initialise every expression
		parsec::logging::loggable{"parsec::expr::seq"},
		__m_expressions{std::tuple_cat(std::move(sequence.expressions()),
									   std::make_tuple(std::move(expr)))}
	{
	}

	/*
	 * Iterate through each expression in seq, and attempt to match the input
	 *
	 */
	auto evaluate(std::string& input) const
		-> const std::unique_ptr<ast::base_node> override
	{
		// Shall contain the string consumed by the evaluating process
		std::ostringstream consumed_oss;

		// Copy the input to avoid accidentally consuming the input if the
		// matching fails
		std::string __input{input};

		logging::debug(console, "evaluating {}", input);

		// NOTE: this evaluate CAN throw exceptions, should input matching fail
		// should this be wrapped in a comparison with __m_success, and throw
		// exception::seq?

		// NOTE: this evaluate CAN modify its input
		evaluate(__input, consumed_oss, std::index_sequence_for<Exprs...>{});
		logging::debug(console, "evaluation yielded {}", consumed_oss.str());

		set_consumed(consumed_oss.str());

		input = __input;
		return std::make_unique<ast::seq<ast::node_from_expr_t<Exprs>...>>(
			*this);
	}

	/*
	 * Override the behaviour of the setter of the consumed string,
	 * by calculating the consumed string by concatenating the consumed
	 * strings of every expressioj stored in this class
	 */
	auto consumed() const -> const std::string override
	{
		std::ostringstream oss;
		return consumed(oss, std::index_sequence_for<Exprs...>{}).str();
	}

	/*
	 * Returns the expressions stored in the class
	 */
	auto expressions() const -> const expr_tuple
	{
		return __m_expressions;
	}

	template <typename Expr1, typename Expr2>
	friend auto operator,(Expr1 expr1, Expr2 expr2) -> seq<Expr1, Expr2>;

	template <typename... PreviousExprs, typename OtherExpr>
	friend auto operator,(seq<PreviousExprs...> sequence, OtherExpr other)
							 -> seq<PreviousExprs..., OtherExpr>;

  private:
	template <std::size_t... Is>
	auto evaluate(std::string& __input,
				  std::ostringstream& oss,
				  std::index_sequence<Is...>) const -> void
	{
		try {
			// clang-format off
			((
				// For every expression stored in this class
				logging::debug(console, "calling std::get<{}>(__m_expressions).evaluate('{}')", Is, __input,
				std::get<Is>(__m_expressions).consumed()),

				// 1. call evaluate on the input
				// Allow possible exceptions to bubble up, i.e. DO NOT CATCH HERE
				std::get<Is>(__m_expressions).evaluate(__input),
				logging::debug(console, "consumed '{}'", std::get<Is>(__m_expressions).consumed()),

				//  2. append the result of the evaluation to the stream
				oss << std::get<Is>(__m_expressions).consumed()
			), ...);
			// clang-format on
		}

		catch (const exception::base_exception& be) {
			logging::debug(console, "{}", be.what());
			throw;
		}

		// all expressions were successfully matched
	}

	/*
	 * Store the consumed string by iterating over every
	 * expression and concatenating the consumed strings in a string stream
	 */
	template <std::size_t... Is>
	auto consumed(std::ostringstream& oss, std::index_sequence<Is...>) const
		-> std::ostringstream&
	{
		// This is guaranteed to write to oss in the correct order
		// as the built-in comma operator guarantees left to right
		// execution (and the overloaded version since C++17)
		((oss << std::get<Is>(__m_expressions).consumed()), ...);
		return oss;
	}

	expr_tuple __m_expressions;
}; // namespace parsec

/*
 * Template deduction guide intended to guarantee compilation across standard
 * conforming compilers
 */
template <typename Expr1, typename Expr2>
seq(Expr1, Expr2)->seq<Expr1, Expr2>;

/*
 * Generate parsec::expr::seq by specifying a series of expressions separated by
 * commas and surrounded by paranthesis, because the precedence of the comma
 * operator is lower than that of the assignment symbol
 */
template <typename Expr1, typename Expr2>
auto operator,(Expr1 expr1, Expr2 expr2) -> seq<Expr1, Expr2>
{
	return seq{std::move(expr1), std::move(expr2)};
}

/*
 * Template deduction guide intended to guarantee compilation across standard
 * conforming compilers
 */
template <typename... PreviousExprs, typename OtherExpr>
seq(seq<PreviousExprs...>, OtherExpr)->seq<PreviousExprs..., OtherExpr>;

/*
 * To avoid instantiation of chained instances of parsec::seq (for example
 * parsec::seq<parsec::expr, parsec::seq<parsec::expr, parsec::expr>>), we
 * generate flattened instances of parsec::seq by appending any expression that
 * follows parsec::seq to that same instance of parsec::seq, so that the result
 * reads parsec::seq<parsec::expr, parsec::expr, parsec::expr> instead
 *
 * If this is not desired behaviour, explicitly call the constructor instead of
 * relying on the overloaded comma operator
 */
template <typename... PreviousExprs, typename OtherExpr>
auto operator,(seq<PreviousExprs...> sequence, OtherExpr other)
				  -> seq<PreviousExprs..., OtherExpr>
{
	// There is no need to check here whether PreviousAsts or OtherExpr is
	// actually made up of purely expressions, because parsec::seq itself checks
	// for this
	return seq{std::move(sequence), std::move(other)};
}
} // namespace parsec::expr

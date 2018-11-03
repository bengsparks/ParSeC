#include <grammar/expressions/base_expr.hxx>

using namespace parsec::expr;

/*
 *  Implement getter and setter for the consumed string
 */
auto base_expr::consumed() const -> const std::string
{
	return __m_consumed;
}

auto base_expr::set_consumed(std::string consumed) const -> void
{
	__m_consumed = std::move(consumed);
}

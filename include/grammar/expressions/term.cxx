#include <ast/term.hxx>

#include <grammar/exceptions/term.hxx>
#include <grammar/expressions/term.hxx>

#include <logging/logging.hxx>

using namespace parsec::expr;

term::term(std::string terminal) :
	// Initialise the logger
	parsec::logging::loggable("parsec::expr::term"),

	// Construct the terminal symbol to be matched
	__m_terminal{std::move(terminal)}
{
	// Special case: empty string i.e. epsilon
	if (__m_terminal.empty()) {
		throw exception::term{
			"Empty terminal symbols are not permitted, use "
			"parsec::expr::epsilon instead"};
	}
}

/*
 * Attempt to evaluate the input string by attempting to match the prefix of the
 * input with the stored terminal symbol
 *
 * If the input string starts with the stored terminal symbol, that very same
 * section of the input string shall be consumed and passed back to the caller,
 * and the corresponding ast node, ast::term, shall be returned, containing the
 * consumed section of the string
 *
 * If the input string does NOT start with the stored symbol, then an
 * exception::term is raised, with a corresponding error message
 */
auto term::evaluate(std::string& input) const
	-> const std::unique_ptr<ast::base_node>
{
	logging::debug(console, "evaluating '{}'", input);

	// check if the input *STARTS WITH* the terminal symbol to be matched
	if (input.compare(0, __m_terminal.size(), __m_terminal) == 0) {
		// set the consumed input
		set_consumed(__m_terminal);
		logging::debug(console, "consumed '{}'", __m_terminal);

		// consume input
		input = input.substr(__m_terminal.size());
		logging::debug(console, "input left: '{}'", input);

		return std::make_unique<ast::term>(*this);
	}

	logging::debug(console, "unable to match '{}' with '{}'", __m_terminal,
				   input);
	logging::debug(console, "consumed nothing, remaining input: '{}'", input);
	throw exception::term(input + " != " + __m_terminal);
}

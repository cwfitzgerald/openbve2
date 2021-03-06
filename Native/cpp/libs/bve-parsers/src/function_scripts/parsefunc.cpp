#include "parse_tree.hpp"
#include "parsers/function_scripts.hpp"

namespace bve::parsers::function_scripts {
	InstructionList parse(const std::string& text) {
		errors::Errors e;
		auto const tokens = lex(text, e);
		auto const tree = create_tree(tokens, e);
		auto instructions = build_instructions(tree, e);
		return instructions;
	}
} // namespace bve::parsers::function_scripts

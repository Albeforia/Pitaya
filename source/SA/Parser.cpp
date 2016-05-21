#include "Parser.h"

namespace pitaya {

	Parser::Parser(Grammar& grammar, ItemSetBuilder& builder)
		: m_grammar {grammar}, m_builder {builder} {}

	void Parser::parse(Tokenizer& tokenizer) {
		while (tokenizer.has_next()) {
			auto& token = tokenizer.next();
			//
		}
	}

}
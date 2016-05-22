#pragma once

#include "ItemSetBuilder.h"
#include "Tokenizer.h"

#include <stack>

namespace pitaya {

	class Parser {

	public:

		//! Constructor.
		Parser(Grammar&, ItemSetBuilder&);

		//! Parse token stream.
		bool parse(Tokenizer&);

	private:

		Grammar& m_grammar;				//!< The grammar.
		ItemSetBuilder& m_builder;		//!< The item-set builder.

		bool evaluate(Action&, std::stack<StateID>&, Tokenizer&);

	};

}
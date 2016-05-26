#pragma once

#include "ItemSetBuilder.h"
#include "Tokenizer.h"

#include <stack>

namespace pitaya {

	/*!
		\ingroup SA
		Parser class.
	*/
	class Parser {

	public:

		//! Constructor.
		Parser(Grammar&, ItemSetBuilder&);

		//! Parse the token stream.
		bool parse(Tokenizer&);

	private:

		Grammar& m_grammar;				//!< The grammar.
		ItemSetBuilder& m_builder;		//!< The item-set builder.

		//! Evaluate state transition against an action.
		bool evaluate(Action&, std::stack<StateID>&, Tokenizer&);

	};

}
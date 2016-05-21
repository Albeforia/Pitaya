#pragma once

#include "ItemSetBuilder.h"
#include "Tokenizer.h"

namespace pitaya {

	class Parser {

	public:

		//! Constructor.
		Parser(Grammar&, ItemSetBuilder&);

		//! Parse token stream.
		void parse(Tokenizer&);

	private:

		Grammar& m_grammar;				//!< The grammar.
		ItemSetBuilder& m_builder;		//!< The item-set builder.

	};

}
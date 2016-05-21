#pragma once

#include "StateBuilder.h"

namespace pitaya {

	struct Token {

		Token(std::size_t index);

		std::size_t token_index;
		std::string value;

	};

	//! Tokenizer class.
	class Tokenizer {

	public:

		//! Constructor.
		Tokenizer(Grammar&, StateBuilder&);

		//! Parse a source file.
		void parse(std::ifstream&);

		//! Get next token.
		const Token& next();

		//! Whether there are any tokens left.
		bool has_next() const;

		/// @cond test
		void print_all() const;
		/// @endcond

	private:

		Grammar& m_grammar;				//!< The grammar.
		StateBuilder& m_builder;		//!< The state builder.
		std::vector<Token> m_tokens;	//!< The token stream.
		std::size_t m_current;			//!< Index of the token stream.

	};

}
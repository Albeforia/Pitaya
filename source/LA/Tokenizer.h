#pragma once

#include "StateBuilder.h"

namespace pitaya {

	struct Token {

		const char* type;
		std::string value;

	};

	//! Tokenizer class.
	class Tokenizer {

	public:

		struct ParseResult {

			bool success;
			std::size_t err_line;
			char err_input;

		};

		//! Constructor.
		Tokenizer(Grammar&, StateBuilder&);

		//! Parse a source file.
		ParseResult parse(std::ifstream&);

		//! Get the next token.
		const Token& next();

		//! Peek the next token.
		const Token& peek();

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
		std::size_t m_curr_line;		//!< Current line number.

		/// @cond
		static auto& pool() {
			static std::unordered_set<std::string> interned_;
			return interned_;
		}
		/// @endcond

	};

}
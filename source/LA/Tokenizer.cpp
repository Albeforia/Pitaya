#include "Tokenizer.h"
#include "Grammar.h"

#include <cctype>
#include <fstream>
#include <iostream>

namespace pitaya {

	Token::Token(std::size_t index)
		: token_index {index}, value {} {}

	Tokenizer::Tokenizer(Grammar& grammar, StateBuilder& builder)
		: m_grammar {grammar}, m_builder {builder}, m_tokens {}, m_current {} {}

	void Tokenizer::parse(std::ifstream& file) {
		while (file.peek() != std::ifstream::traits_type::eof()) {
			while (file.peek() == '\n' || isspace(file.peek())) {
				file.get();
			}
			if (file.peek() != std::ifstream::traits_type::eof()) {
				// restart
				auto state = &m_builder.get_state(1);
				auto symbol = &m_grammar.get_symbol(std::string(1, char(file.peek())));
				if (*symbol == m_grammar.endmark()) {
					// error: undefined symbol
					return;
				}
				if (symbol->type() == SymbolType::MULTITERMINAL) {
					symbol = &symbol->shared_terminal();
				}
				auto start_pos = file.tellg();
				auto parse_pos = start_pos;
				auto last_final_pos = parse_pos;
				State::ID next_state = 0, last_final = 0;
				if (state->is_final()) {
					last_final = state->id();
				}
				while (state->transit(*symbol, next_state)) {
					state = &m_builder.get_state(next_state);
					if (state->is_final()) {
						last_final = state->id();
						last_final_pos = parse_pos;
					}
					// next symbol
					parse_pos += 1;
					file.seekg(parse_pos);
					if (file.peek() == std::ifstream::traits_type::eof()
						|| file.peek() == '\n' || isspace(file.peek())) {
						break;
					}
					symbol = &m_grammar.get_symbol(std::string(1, char(file.peek())));
					if (*symbol == m_grammar.endmark()) {
						// error: undefined symbol
						return;
					}
					if (symbol->type() == SymbolType::MULTITERMINAL) {
						symbol = &symbol->shared_terminal();
					}
				}
				if (last_final == 0) {
					// not in a final state
					return;
				}
				else {
					// recognize a token
					auto index = m_builder.get_state(last_final).token_index();
					m_tokens.emplace_back(index);
					auto& new_token = m_tokens.back();
					file.seekg(start_pos);
					while (file.tellg() <= last_final_pos) {
						new_token.value.append(1, char(file.get()));
					}
					auto& self = m_grammar.get_symbol(new_token.value);
					if (self != m_grammar.endmark() && self.is_token()) {
						if (self.precedence() > m_grammar.get_symbol(index).precedence()) {
							new_token.token_index = self.index();
						}
					}
				}
			}
		}
	}

	const Token& Tokenizer::next() {
		return m_tokens[m_current++];
	}

	bool Tokenizer::has_next() const {
		return m_current >= m_tokens.size();
	}

	void Tokenizer::print_all() const {
		for (auto& token : m_tokens) {
			std::cout << "(" << m_grammar.get_symbol(token.token_index) << ", ";
			for (auto& c : token.value) {
				std::cout << c;
			}
			std::cout << ")" << std::endl;
		}
	}

}
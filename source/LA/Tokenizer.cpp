#include "Tokenizer.h"
#include "Grammar.h"

#include <cctype>
#include <fstream>
#include <iostream>

namespace pitaya {

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
					// error
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
						// error
						return;
					}
					if (symbol->type() == SymbolType::MULTITERMINAL) {
						symbol = &symbol->shared_terminal();
					}
				}
				if (last_final == 0) {
					// error
				}
				else {
					// recognize a token
					m_tokens.emplace_back();
					auto& new_token = m_tokens.back();
					new_token.token_index = m_builder.get_state(last_final).token_index();
					file.seekg(start_pos);
					while (file.tellg() <= last_final_pos) {
						new_token.value.push_back(char(file.get()));
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
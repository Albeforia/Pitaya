#include "Tokenizer.h"
#include "Grammar.h"

#include <cctype>
#include <cassert>
#include <fstream>
#include <iostream>

namespace pitaya {

	Tokenizer::Tokenizer(Grammar& grammar, StateBuilder& builder)
		: m_grammar {grammar}, m_builder {builder}, m_tokens {},
		m_current {}, m_curr_line {1} {}

	Tokenizer::ParseResult Tokenizer::parse(std::ifstream& file) {
		while (file.peek() != std::ifstream::traits_type::eof()) {
			// eat whitespace characters
			while (isspace(file.peek())) {
				if (file.peek() == '\n') {
					m_curr_line++;
				}
				file.get();
			}
			if (file.peek() != std::ifstream::traits_type::eof()) {
				// restart
				auto state = &m_builder.get_state(1);
				char input = file.peek();
				auto symbol = &m_grammar.get_symbol(std::string(1, input));
				if (*symbol == m_grammar.endmark()) {
					// error: undefined symbol
					return ParseResult {false, m_curr_line, input};
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
					if (file.peek() == std::ifstream::traits_type::eof() || isspace(file.peek())) {
						break;
					}
					input = file.peek();
					symbol = &m_grammar.get_symbol(std::string(1, input));
					if (*symbol == m_grammar.endmark()) {
						// error: undefined symbol
						return ParseResult {false, m_curr_line, input};
					}
					if (symbol->type() == SymbolType::MULTITERMINAL) {
						symbol = &symbol->shared_terminal();
					}
				}
				if (last_final == 0) {
					// err: not in a final state
					return ParseResult {false, m_curr_line, input};
				}
				else {
					// recognize a token
					m_tokens.emplace_back();
					auto& new_token = m_tokens.back();
					file.seekg(start_pos);
					while (file.tellg() <= last_final_pos) {
						new_token.value.append(1, char(file.get()));
					}
					auto index = m_builder.get_state(last_final).token_index();
					auto& self = m_grammar.get_symbol(new_token.value);
					// check token precedence(e.g. key-words)
					if (self != m_grammar.endmark() && self.is_token()) {
						if (self.precedence() > m_grammar.get_symbol(index).precedence()) {
							index = self.index();
						}
					}
					assert(index != 0);		// token must be defined
					new_token.type = pool().emplace(m_grammar.get_symbol(index).name()).first->c_str();
				}
			}
		}
		return ParseResult {true};
	}

	const Token& Tokenizer::next() {
		return m_tokens[m_current++];
	}

	const Token& Tokenizer::peek() {
		return m_tokens[m_current];
	}

	bool Tokenizer::has_next() const {
		return m_current < m_tokens.size();
	}

	void Tokenizer::print_all() const {
		for (auto& token : m_tokens) {
			std::cout << "(" << token.type << "\t"
				<< token.value << ")" << std::endl;
		}
	}

}
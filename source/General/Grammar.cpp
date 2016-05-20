#include "Grammar.h"

#include <fstream>
#include <algorithm>
#include <iostream>
#include <cctype>

namespace pitaya {

	Grammar::Grammar(const char* file)
		: m_productions {}, m_symbols {} {
		Symbol::create("$");
		read(file);
		rearrange_symbols();
		rearrange_productions();
	}

	std::size_t Grammar::symbol_count() const {
		return m_symbols.size();
	}

	std::size_t Grammar::terminal_count() const {
		return m_terminal_count;
	}

	std::size_t Grammar::production_count() const {
		return m_productions.size();
	}

	Symbol& Grammar::get_symbol(SymbolID id) {
		return *m_symbols[id];
	}

	Symbol& Grammar::get_symbol(std::string name) {
		return *Symbol::pool().at(name);
	}

	Production& Grammar::get_production(ProductionID id) {
		return m_productions[id];
	}

	Grammar::PP Grammar::productions_by_lhs(SymbolID id) {
		return m_productions_by_lhs.at(id);
	}

	Symbol& Grammar::endmark() {
		return *m_symbols[0];
	}

	void Grammar::rearrange_symbols() {
		// after reading grammar file, every nonterminal has been determined
		for (const auto& p : Symbol::pool()) {
			if (p.second->m_type == SymbolType::UNDEFINED) {
				p.second->m_type = SymbolType::TERMINAL;
			}
			m_symbols.emplace_back(p.second);
		}

		// sort terminals before nonterminals while
		// keeping the order they appeared in the grammar file
		std::sort(m_symbols.begin(), m_symbols.end(), [](auto& a, auto& b) {
			if (a->type() != b->type()) {
				return a->type() == SymbolType::TERMINAL;
			}
			else {
				return a->id() < b->id();
			}
		});

		// reset symbol's id
		m_terminal_count = 0;
		std::size_t nonterminal_count = 0;
		for (auto& s : m_symbols) {
			if (s->type() == SymbolType::TERMINAL) {
				s->m_id = m_terminal_count++;
			}
			else {
				s->m_id = m_terminal_count + nonterminal_count++;
				s->first_set().resize(m_terminal_count);
			}
		}
	}

	void Grammar::rearrange_productions() {
		assert(m_productions.size() > 0);

		// sort productions by lhs then by id
		std::sort(m_productions.begin(), m_productions.end(), [](auto& a, auto& b) {
			if (a[0] == b[0]) {
				return a.id() < b.id();
			}
			else {
				return a[0].id() < b[0].id();
			}
		});

		auto curr_lhs = m_productions[0][0].id();
		std::size_t start = 0;
		for (std::size_t i = 0; i < m_productions.size(); i++) {
			if (m_productions[i][0].id() != curr_lhs) {
				m_productions_by_lhs.emplace(curr_lhs, PP {start, i - 1});
				curr_lhs = m_productions[i][0].id();
				start = i;
			}
			m_productions[i].m_id = i;
		}
		// the last group
		m_productions_by_lhs.emplace(curr_lhs, PP {start, m_productions.size() - 1});
	}

	void Grammar::read(const char* file) {
		std::ifstream f;
		f.open(file);
		if (f.is_open()) {
			// eat '\n's
			while (f.peek() == '\n') {
				f.get();
			}
			ProductionReader reader;
			reader.initiate();
			while (f.peek() != std::ifstream::traits_type::eof()) {
				reader.read(f, m_productions);
			}
			reader.terminate();
		}
		f.close();
	}

	void Grammar::print_first_sets() const {
		std::cout << "FIRST sets:" << std::endl;
		for (const auto& s : m_symbols) {
			if (s->type() == SymbolType::NONTERMINAL) {
				std::cout << *s << ":\t";
				for (SymbolID i = 0; i < s->first_set().size(); i++) {
					if (s->first_set()[i]) {
						std::cout << *m_symbols[i] << " ";
					}
				}
				if (s->lambda()) {
					std::cout << "(empty)";
				}
				std::cout << std::endl;
			}
		}
	}

	Grammar::ProductionReader::ProductionReader()
		: curr_pid {}, curr_assc {}, curr_prec {-1}, token_decl {false} {}

	void Grammar::ProductionReader::read(std::ifstream& file,
										 std::vector<pitaya::Production>& productions) {
		if (state_downcast<const WaitForLHS*>() != nullptr) {
			if (file.peek() == '%') {
				file.get();
				process_event(EvStartDecl {});
			}
			else {
				std::string lhs;
				file >> lhs;
				productions.emplace_back(curr_pid, Symbol::create(lhs));
				productions[curr_pid][0].m_type = SymbolType::NONTERMINAL;
				process_event(EvGetSymbol {});
			}
		}
		else if (state_downcast<const WaitForDecl*>() != nullptr) {
			if (std::isalpha(file.peek())) {
				std::string s;
				file >> s;
				curr_assc = Associativity::UNDEFINED;
				if (s == "left") curr_assc = Associativity::LEFT;
				if (s == "right") curr_assc = Associativity::RIGHT;
				if (s == "none") curr_assc = Associativity::NONE;
				if (s == "token") token_decl = true;
				if (curr_assc != Associativity::UNDEFINED || token_decl) {
					curr_prec++;
					process_event(EvStartDecl {});
				}
			}
			process_event(EvStartComm {});
		}
		else if (state_downcast<const ReadComment*>() != nullptr) {
			while (file.peek() != '\n' && file.peek() != std::ifstream::traits_type::eof()) {
				file.get();
			}
			// eat '\n's
			while (file.peek() == '\n') {
				file.get();
			}
			process_event(EvNextProduction {});
		}
		else if (state_downcast<const WaitForSyms*>() != nullptr) {
			if (file.peek() == '\n') {
				// eat '\n's
				while (file.peek() == '\n') {
					file.get();
				}
				token_decl = false;
				process_event(EvNextProduction {});
			}
			else {
				std::string s;
				file >> s;
				auto& sym = Symbol::create(s);
				sym->m_associativity = curr_assc;
				sym->m_precedence = curr_prec;
				if (token_decl) {
					sym->is_token() = true;
				}
				process_event(EvGetSymbol {});
			}
		}
		else {
			if (file.peek() == '\n') {
				// eat '\n's
				while (file.peek() == '\n') {
					file.get();
				}
				curr_pid++;
				process_event(EvNextProduction {});
			}
			else {
				std::string rhs;
				file >> rhs;
				productions[curr_pid].m_rhs.emplace_back(std::move(Symbol::create(rhs)));
				process_event(EvGetSymbol {});
			}
		}
	}

	sc::result Grammar::WaitForLHS::react(const EvGetSymbol& ev) {
		return transit<WaitForRHS>();
	}

	sc::result Grammar::WaitForLHS::react(const EvStartDecl& ev) {
		return transit<WaitForDecl>();
	}

	sc::result Grammar::WaitForRHS::react(const EvGetSymbol& ev) {
		return discard_event();
	}

	sc::result Grammar::WaitForDecl::react(const EvStartDecl& ev) {
		return transit<WaitForSyms>();
	}

	sc::result Grammar::WaitForDecl::react(const EvStartComm& ev) {
		return transit<ReadComment>();
	}

	sc::result Grammar::WaitForSyms::react(const EvGetSymbol& ev) {
		return discard_event();
	}

}
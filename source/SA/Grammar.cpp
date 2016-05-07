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

		for (const auto& p : Symbol::pool()) {
			if (p.second->type() == SymbolType::UNDEFINED) {
				p.second->type() = SymbolType::TERMINAL;
			}
			m_symbols.emplace_back(p.second);
		}

		std::sort(m_symbols.begin(), m_symbols.end(), [](auto& a, auto& b) {
			// sort terminals before nonterminals
			// while keeping the order they appeared in the grammar file
			if (a->type() != b->type()) {
				return a->type() == SymbolType::TERMINAL;
			}
			else {
				return a->id() < b->id();
			}
		});

		m_terminal_count = 0;
		std::size_t nonterminal_count = 0;
		for (auto& s : m_symbols) {
			if (s->type() == SymbolType::TERMINAL) {
				s->id() = m_terminal_count++;
			}
			else {
				s->id() = m_terminal_count + nonterminal_count++;
				s->first_set().resize(m_terminal_count);
			}
		}
		assert(m_terminal_count + nonterminal_count == m_symbols.size());

		assert(m_productions.size() > 0);

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
			m_productions[i].id() = i;
		}
		m_productions_by_lhs.emplace(curr_lhs, PP {start, m_productions.size() - 1});
	}

	std::size_t Grammar::terminal_count() const {
		return m_terminal_count;
	}

	Symbol& Grammar::get_symbol(SymbolID id) {
		return *m_symbols[id];
	}

	Production& Grammar::get_production(ProductionID id) {
		return m_productions[id];
	}

	Grammar::PP Grammar::productions_by_lhs(SymbolID id) {
		return m_productions_by_lhs.at(id);
	}

	std::size_t Grammar::production_count() const {
		return m_productions.size();
	}

	Symbol& Grammar::endmark() {
		return *m_symbols[0];
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

	void Grammar::compute_first_sets() {
		bool not_fin = false;

		// compute all lambdas
		do {
			not_fin = false;
			for (auto& p : m_productions) {
				if (p[0].lambda()) continue;
				std::size_t i = 0;
				for (i; i < p.rhs_count(); i++) {
					auto& rhs = p[i + 1];
					assert(rhs.type() == SymbolType::NONTERMINAL || !rhs.lambda());
					if (!rhs.lambda()) break;		// lhs is lambda <=> all rhs are lambda
				}
				if (i == p.rhs_count()) {				// including zero rhs
					p[0].lambda() = true;
					not_fin = true;					// find a new lambda, continue computing
				}
			}
		} while (not_fin);

		// compute all first sets
		do {
			not_fin = false;
			for (auto& p : m_productions) {
				auto& lhs = p[0];
				for (std::size_t i = 0; i < p.rhs_count(); i++) {
					auto& rhs = p[i + 1];
					if (rhs.type() == SymbolType::TERMINAL) {
						not_fin = lhs.first_set().add(rhs);
						break;		// encounter a terminal, add to first set and stop
					}
					else if (lhs == rhs) {
						if (!lhs.lambda()) break;	// recurrence happened, should compute in another production
					}
					else {
						not_fin = lhs.first_set().union_with(rhs.first_set());
						if (!rhs.lambda()) break;	// stop if a rhs cannot generate empty string
					}
				}
			}
		} while (not_fin);
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
		: curr_pid {}, curr_assc {}, curr_prec {-1} {}

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
				productions[curr_pid][0].type() = SymbolType::NONTERMINAL;
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
				if (curr_assc != Associativity::UNDEFINED) {
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
				process_event(EvNextProduction {});
			}
			else {
				std::string s;
				file >> s;
				auto& sym = Symbol::create(s);
				sym->associativity() = curr_assc;
				sym->precedence() = curr_prec;
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
				productions[curr_pid].rhs().emplace_back(std::move(Symbol::create(rhs)));
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
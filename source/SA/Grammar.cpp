#include "Grammar.h"

#include <fstream>
#include <algorithm>
#include <iostream>

namespace pitaya {

	Grammar::Grammar(const char* file)
		: m_productions {}, m_symbols {} {
		read(file);

		for (const auto& p : Symbol::pool()) {
			m_symbols.emplace_back(p.second);
		}

		std::sort(m_symbols.begin(), m_symbols.end(), [](auto a, auto b) {
			// sort terminals before nonterminals
			// while keeping the order they appeared in the grammar file
			if (a->type() != b->type()) {
				return a->type() == SymbolType::TERMINAL;
			}
			else {
				return a->id() < b->id();
			}
		});

		std::size_t num_terminals = 0;
		std::size_t num_nonterminals = 0;
		for (auto& s : m_symbols) {
			if (s->type() == SymbolType::TERMINAL) {
				s->id() = num_terminals++;
			}
			else {
				s->id() = num_terminals + num_nonterminals++;
				s->first_set().resize(num_terminals + 1);
			}
		}
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
				std::size_t i;
				for (i = 0; i < p.num_rhs(); i++) {
					auto& rhs = p[i + 1];
					assert(rhs.type() == SymbolType::NONTERMINAL || !rhs.lambda());
					if (!rhs.lambda()) break;		// lhs is lambda <=> all rhs are lambda
				}
				if (i == p.num_rhs()) {				// including zero rhs
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
				for (std::size_t i = 0; i < p.num_rhs(); i++) {
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
				for (size_t i = 0; i < s->first_set().size(); i++) {
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
		: current {} {}

	void Grammar::ProductionReader::read(std::ifstream& file,
										 std::vector<pitaya::Production>& productions) {
		if (state_downcast<const WaitForLHS*>() != nullptr) {
			std::string lhs;
			file >> lhs;
			productions.emplace_back(Production {});
			productions[current].set_lhs(Symbol::create(lhs));
			process_event(EvGetLHS {});
		}
		else {
			if (file.peek() == '\n') {
				// eat '\n's
				while (file.peek() == '\n') {
					file.get();
				}
				current++;
				process_event(EvNextProduction {});
			}
			else {
				std::string rhs;
				file >> rhs;
				productions[current].rhs().emplace_back(std::move(Symbol::create(rhs)));
				process_event(EvGetRHS {});
			}
		}
	}

	sc::result Grammar::WaitForLHS::react(const EvGetLHS& ev) {
		return transit<WaitForRHS>();
	}

	sc::result Grammar::WaitForRHS::react(const EvNextProduction& ev) {
		return transit<WaitForLHS>();
	}

	sc::result Grammar::WaitForRHS::react(const EvGetRHS& ev) {
		return discard_event();
	}

}
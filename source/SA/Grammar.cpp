#include "Grammar.h"

#include <fstream>

namespace pitaya {

	Grammar::Grammar(const char* file)
		: m_productions {}, m_symbols {
		[](SharedSymbol a, SharedSymbol b) {
			// sort terminals before nonterminals
			// while keeping the order they appeared in the grammar file
			if (a->type() != b->type()) {
				return a->type() == SymbolType::TERMINAL;
			}
			else {
				return a->id() < b->id();
			}
		}
	} {
		read(file);

		for (const auto& p : Symbol::pool()) {
			auto& res = m_symbols.emplace(p.second);
			// the insertion should success because we fetch from pool
			assert(res.second);
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
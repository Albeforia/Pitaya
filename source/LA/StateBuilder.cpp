#include "StateBuilder.h"

#include <cassert>
#include <iostream>

namespace pitaya {

	StateBuilder::StateBuilder(std::shared_ptr<Grammar>& grammar)
		: m_grammar {grammar}, m_states {}, m_curr_state {} {}

	void StateBuilder::build() {
		// initial state
		m_curr_state.reset();
		// assume the start symbol is the lhs of the first production
		m_curr_state.add_symbol(m_grammar->get_production(0)[0]);
		build_state();
		decide_token_type();
	}

	const State& StateBuilder::build_state() {
		m_curr_state.sort();		// sort basis for hashing and comparing
		auto& find = m_states.find(m_curr_state);
		if (find != m_states.end()) {
			// a state with the same basis already exists
			m_curr_state.reset();
			return *find;
		}
		// a new state
		// compute closure before moving
		m_curr_state.compute_closure(*m_grammar);
		// 'move' the currently building state into the new state
		auto& new_item = m_states.emplace(std::move(m_curr_state)).first;
		// compute successors
		build_successors(*new_item);
		return *new_item;
	}

	void StateBuilder::build_successors(const State& state) {
		bool has_transition = false;
		// for every terminal
		for (SymbolID tid = 0; tid < m_grammar->terminal_count(); tid++) {
			has_transition = false;
			auto& symbol = m_grammar->get_symbol(tid);
			// for every nonterminal in the closure
			for (auto ntid = m_grammar->terminal_count(); ntid < m_grammar->symbol_count(); ntid++) {
				if (!state.closure()[ntid]) continue;
				// for each production with symbol of ntid as its lhs
				auto range = m_grammar->productions_by_lhs(ntid);
				for (auto pid = range.first; pid <= range.second; pid++) {
					auto& p = m_grammar->get_production(pid);
					assert(p.rhs_count() <= 2);		// assert g is a regular grammar
					if (p.rhs_count() == 1) {
						if (p[1] != symbol) continue;
						// transit to final state
						m_curr_state.add_symbol(m_grammar->endmark());
						m_curr_state.is_final() = true;
						has_transition = true;
					}
					else if (p.rhs_count() == 2) {
						if (p[1] != symbol) continue;
						assert(p[2].type() == SymbolType::NONTERMINAL);
						m_curr_state.add_symbol(p[2]);
						has_transition = true;
					}
				}
			}
			if (has_transition) {
				auto& new_state = build_state();
				state.add_transition(symbol.id(), new_state.id());
			}
		}
	}

	void StateBuilder::decide_token_type() {
		for (auto& state : m_states) {
			if (!state.is_final()) continue;
			int prec = -1;
			// for every nonterminal in the state's closure
			for (auto i = m_grammar->terminal_count(); i < m_grammar->symbol_count(); i++) {
				if (state.closure()[i]) {
					auto& s = m_grammar->get_symbol(i);
					if (!s.is_token()) continue;
					if (s.precedence() > prec) {
						state.token_type() = s.id();
						prec = s.precedence();
					}
				}
			}
		}
	}

	void StateBuilder::print_all() const {
		for (auto& state : m_states) {
			std::cout << state.id() << ":\t" << "{ ";
			for (SymbolID sid = 0; sid < m_grammar->symbol_count(); sid++) {
				if (state.closure()[sid]) {
					std::cout << m_grammar->get_symbol(sid) << " ";
				}
			}
			std::cout << "}";
			if (state.is_final()) {
				std::cout << " [final]";
			}
			std::cout << std::endl;
			State::ID to;
			for (SymbolID sid = 0; sid < m_grammar->symbol_count(); sid++) {
				auto& symbol = m_grammar->get_symbol(sid);
				if (state.transit(symbol, to)) {
					std::cout << "\t" << symbol << " --> " << to << std::endl;
				}
			}
			std::cout << std::endl;
		}
	}

}
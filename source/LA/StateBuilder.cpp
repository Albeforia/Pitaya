#include "StateBuilder.h"
#include "Grammar.h"

#include <cassert>
#include <algorithm>
#include <iostream>

namespace pitaya {

	StateBuilder::StateBuilder(Grammar& grammar)
		: m_grammar {grammar}, m_states {}, m_sorted {}, m_curr_state {} {}

	void StateBuilder::build() {
		// initial state
		m_curr_state.reset();
		// assume the start symbol is the lhs of the start production
		m_curr_state.add_symbol(m_grammar.get_production(0)[0]);
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
		m_curr_state.compute_closure(m_grammar);
		// 'move' the currently building state into the new state
		auto& new_item = m_states.emplace(std::move(m_curr_state)).first;
		// compute successors
		build_successors(*new_item);
		return *new_item;
	}

	void StateBuilder::build_successors(const State& state) {
		bool has_transition = false;
		// for every terminal
		for (auto it = m_grammar.terminal_begin(); it != m_grammar.terminal_end(); it++) {
			has_transition = false;
			auto& symbol = **it;
			// for every nonterminal in the closure
			for (auto it2 = m_grammar.nonterminal_begin(); it2 != m_grammar.nonterminal_end(); it2++) {
				if (!state.closure()[**it2]) continue;
				// for each production with symbol of ntid as its lhs
				auto range = m_grammar.productions_by_lhs(**it2);
				for (auto pid = range.first; pid <= range.second; pid++) {
					auto& p = m_grammar.get_production(pid);
					assert(p.rhs_count() <= 2);		// assert g is a regular grammar
					if (p.rhs_count() == 1) {
						if (p[1] != symbol) continue;
						// transit to final state
						m_curr_state.add_symbol(m_grammar.endmark());
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
				state.add_transition(symbol, new_state);
			}
		}
	}

	void StateBuilder::decide_token_type() {
		m_sorted.resize(m_states.size() + 1);
		std::fill(m_sorted.begin(), m_sorted.end(), nullptr);
		for (auto& state : m_states) {
			m_sorted[state.id()] = &state;
			if (!state.is_final()) continue;
			int prec = -1;
			// for every nonterminal in the state's closure
			for (auto it = m_grammar.nonterminal_begin(); it != m_grammar.nonterminal_end(); it++) {
				if (state.closure()[**it]) {
					auto& s = **it;
					if (!s.is_token()) continue;
					if (s.precedence() > prec) {
						state.token_index() = s.index();
						prec = s.precedence();
					}
				}
			}
			assert(prec != -1);
		}
	}

	const State& StateBuilder::get_state(State::ID id) const {
		assert(id > 0);		// avoid nullptr
		return *m_sorted[id];
	}

	void StateBuilder::print_all() const {
		for (auto& p : m_sorted) {
			if (p == nullptr) continue;
			auto& state = *p;
			std::cout << state.id() << ":\t" << "{ ";
			for (auto it = m_grammar.nonterminal_begin(); it != m_grammar.nonterminal_end(); it++) {
				if (state.closure()[**it]) {
					std::cout << **it << " ";
				}
			}
			std::cout << "}";
			if (state.is_final()) {
				std::cout << " [final]";
			}
			std::cout << std::endl;
			State::ID to;
			for (auto it = m_grammar.terminal_begin(); it != m_grammar.terminal_end(); it++) {
				if (state.transit(**it, to)) {
					std::cout << "\t" << **it << " --> " << to << std::endl;
				}
			}
			std::cout << std::endl;
		}
	}

}
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
		m_curr_state.add_base(m_grammar.get_production(0)[0], 0);
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
		// for every symbol(include multi-terminal)
		for (auto it = m_grammar.symbol_begin(); it != m_grammar.symbol_end(); it++) {
			has_transition = false;
			auto& symbol = **it;
			if (symbol.type() == SymbolType::NONTERMINAL) continue;
			// for every item in the closure
			for (auto& item : state.m_closure) {
				// item: <symbol_index, token_index>
				auto& left = m_grammar.get_symbol(item.first);
				if (left.type() != SymbolType::NONTERMINAL) continue;
				auto range = m_grammar.productions_by_lhs(left);
				for (auto pid = range.first; pid <= range.second; pid++) {
					auto& p = m_grammar.get_production(pid);
					if (p.rhs_count() == 0) continue;
					if (p[1] != symbol) continue;
					has_transition = true;
					if (p.rhs_count() == 1) {
						// A -> a, transit to a final state
						m_curr_state.is_final() = true;
						m_curr_state.add_base(symbol, item.second);
						m_curr_state.m_final_index = symbol.index();
					}
					else {
						// A -> aB
						if (p[2].type() == SymbolType::NONTERMINAL) {
							m_curr_state.add_base(p[2], item.second);
						}
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
			for (auto& item : state.m_closure) {
				if (state.m_final_index == item.first) {
					state.token_index() = item.second;
					break;
				}
			}
			//assert(prec != -1);
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
			std::cout << state.id() << ":\t" << std::endl;
			for (auto& item : state.m_closure) {
				std::cout << "<" << m_grammar.get_symbol(item.first) << "\t"
					<< m_grammar.get_symbol(item.second) << ">" << std::endl;
			}
			if (state.is_final()) {
				std::cout << "[final]\t" << m_grammar.get_symbol(state.token_index());
			}
			std::cout << std::endl;
			State::ID to;
			for (auto it = m_grammar.symbol_begin(); it != m_grammar.symbol_end(); it++) {
				if (state.transit(**it, to)) {
					std::cout << "\t" << **it << " --> " << to << std::endl;
				}
			}
			std::cout << std::endl;
		}
	}

}
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
		m_curr_state.add_base(m_grammar.get_production(0));
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
			auto& eat = **it;
			// for every item in the closure
			for (auto& item : state.closure()) {
				auto& production = m_grammar.get_production(item.production_id());
				auto dot = item.dot();
				// skip item whose dot is at right end
				if (dot >= production.rhs_count()) {
					continue;
				}
				auto& symbol = production[dot + 1];		// symbol after dot
				if (symbol != eat) continue;
				has_transition = true;
				m_curr_state.add_base(production, dot + 1);
			}
			if (has_transition) {
				auto& new_state = build_state();
				state.add_transition(eat, new_state);
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
			// for every item in the state's closure
			for (auto& item : state.closure()) {
				auto& p = m_grammar.get_production(item.production_id());
				if (item.dot() >= p.rhs_count()) {
					auto& s = p[0];
					if (!s.is_token()) continue;
					if (s.precedence() > prec) {
						state.token_index() = s.index();
						prec = s.precedence();
					}
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
			std::cout << state.id() << ":" << std::endl;
			for (auto& item : state.closure()) {
				auto& p = m_grammar.get_production(item.production_id());
				std::cout << "\t" << p[0] << " -> ";
				std::size_t i = 0;
				for (i; i < p.rhs_count(); i++) {
					if (item.dot() == i) {
						std::cout << ".";
					}
					std::cout << p[i + 1] << " ";
				}
				if (item.dot() == i) {
					std::cout << ".";
				}
				std::cout << std::endl;
			}
			std::cout << ">>";
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
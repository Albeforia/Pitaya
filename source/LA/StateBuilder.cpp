#include "StateBuilder.h"
#include "Grammar.h"

#include <cassert>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <regex>

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
					/*
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
					*/
					// FIXME so tricky
					// A -> aB
					if (p.rhs_count() == 2 && p[2].type() == SymbolType::NONTERMINAL) {
						m_curr_state.add_base(p[2], item.second);
					}
					// assert all left productions have the form A -> a[b...x]
					else {
						m_curr_state.add_base(p[1], item.second);
						const State* p_state = &m_curr_state;
						auto p_symbol = &p[1];
						std::string name(p[1].name());		// name = "a"
						// from b to x
						for (size_t i = 1; i < p.rhs_count(); i++) {
							name += p[i + 1].name();
							p_symbol = &m_grammar.get_symbol(name);
							State tmp {};
							tmp.add_base(*p_symbol, item.second);
							tmp.m_closure.emplace(p_symbol->index(), item.second);
							auto& ns = *m_states.emplace(std::move(tmp)).first;
							p_state->add_transition(p[i + 1], ns);
							p_state = &ns;
						}
						p_state->m_is_final = true;
						p_state->m_final_index = p_symbol->index();
					}
				}
			}
			if (has_transition && m_curr_state.m_basis.size() > 0) {
				auto& new_state = build_state();
				state.add_transition(symbol, new_state);
			}
		}
	}

	void StateBuilder::decide_token_type() {
		for (auto& state : m_states) {
			m_sorted.emplace(state.m_id, &state);
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
		return *m_sorted.at(id);
	}

	void StateBuilder::report(bool graph) const {
		std::ofstream file, gfile;
		file.open("report\\lexical_states", std::ios::trunc);
		if (graph) {
			gfile.open("report\\graph\\lexical.dot", std::ios::trunc);
		}
		if (file.is_open()) {
			if (gfile.is_open()) {
				gfile << "digraph lexical_graph {\n" << "node [shape=record];\n";
			}
			for (auto& p : m_sorted) {
				auto& state = *p.second;
				file << "[state " << state.m_id << "]";
				if (state.is_final()) {
					file << " FIN> " << m_grammar.get_symbol(state.token_index());
				}
				file << '\n';

				auto cols = 3, col = 0;
				std::string gitems;
				for (auto& item : state.m_closure) {
					std::string items;
					std::string first(m_grammar.get_symbol(item.first).name());
					std::string second(m_grammar.get_symbol(item.second).name());
					items.append("( ").append(first)
						.append("  ").append(second)
						.append(" )");
					file << std::setw(40) << std::left << items;
					if (++col == cols) {
						col = 0;
						file << '\n';
					}

					std::regex re("[|<>{}]");
					first = std::regex_replace(first, re, "\\$&");
					second = std::regex_replace(second, re, "\\$&");
					gitems.append("{ ").append(first)
						.append(" | ").append(second)
						.append(" }|");
				}
				if (col != 0) {
					file << '\n';
				}
				if (gfile.is_open()) {
					if (gitems.size() > 0) {
						// remove the last '|'
						gitems.erase(gitems.size() - 1);
					}
					gfile << state.m_id << " [label=\"" << state.m_id << "|{"
						<< gitems << "}\"";
					if (state.is_final()) {
						gfile << ", style=bold";
					}
					gfile << "];\n";
				}

				State::ID to;
				for (auto it = m_grammar.symbol_begin(); it != m_grammar.symbol_end(); it++) {
					if (state.transit(**it, to)) {
						file << ">\t" << std::setw(20)
							<< **it << "-->\t" << to << '\n';
						if (gfile.is_open()) {
							gfile << state.m_id << " -> " << to
								<< " [label=\"" << **it << "\"];\n";
						}
					}
				}
				file << '\n';
			}
			if (gfile.is_open()) {
				gfile << "}\n";
			}
		}
		file.close();
		if (graph) {
			gfile.close();
		}
	}

}
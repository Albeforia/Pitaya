#include "State.h"
#include "Grammar.h"

#include <cassert>
#include <algorithm>

#include <boost\functional\hash\hash.hpp>

namespace pitaya {

	State::State()
		: m_id {order()}, m_basis {}, m_closure {},
		m_is_final {false}, m_transitions {},
		m_token_index {}, m_final_index {} {}

	State::State(State&& from) noexcept
		: m_id {order()}, m_is_final {from.m_is_final},
		m_token_index {from.m_token_index}, m_final_index {from.m_final_index},
		m_basis {std::move(from.m_basis)},
		m_closure {std::move(from.m_closure)},
		m_transitions {std::move(from.m_transitions)} {
		// ensure 'from' is empty after move
		from.reset();
	}

	State::ID State::id() const {
		return m_id;
	}

	void State::add_base(const Symbol& symbol, std::size_t token_index) {
		m_basis.emplace_back(symbol.index(), token_index);
	}

	void State::compute_closure(Grammar& grammar) {
		// copy basis into closure
		for (auto& i : m_basis) {
			m_closure.emplace(i.first, i.second);
		}

		bool not_fin = false;
		do {
			not_fin = false;
			for (auto& item : m_closure) {
				// item: <symbol_index, token_index>
				auto& symbol = grammar.get_symbol(item.first);
				if (symbol.type() != SymbolType::NONTERMINAL) continue;
				auto range = grammar.productions_by_lhs(symbol);
				for (auto pid = range.first; pid <= range.second; pid++) {
					auto& p = grammar.get_production(pid);
					// A -> B
					if (p.rhs_count() == 1 && p[1].type() == SymbolType::NONTERMINAL) {
						not_fin = m_closure.emplace(p[1].index(), p[1].index()).second;
					}
					// A -> ε, this is a final state
					else if (p.rhs_count() == 0) {
						m_is_final = true;
						m_final_index = p[0].index();
					}
				}
			}
		} while (not_fin);
	}

	bool& State::is_final() const {
		return m_is_final;
	}

	std::size_t& State::token_index() const {
		return m_token_index;
	}

	void State::add_transition(const Symbol& symbol, const State& state) const {
		auto& res = m_transitions.emplace(symbol.rank, state.m_id);
		if (!res.second) {
			// duplicate transition
			assert(res.first->second == state.m_id);		// DFA assertion
		}
	}

	bool State::transit(const Symbol& symbol, ID& to) const {
		auto& find = m_transitions.find(symbol.rank);
		if (find != m_transitions.end()) {
			to = find->second;
			return true;
		}
		return false;
	}

	bool State::transit(const Symbol& symbol) const {
		auto& find = m_transitions.find(symbol.rank);
		return find != m_transitions.end();
	}

	void State::sort() {
		std::sort(m_basis.begin(), m_basis.end());
	}

	void State::reset() {
		m_basis.clear();
		m_closure.clear();
		m_transitions.clear();
		m_is_final = false;
		m_token_index = 0;
		m_final_index = 0;
	}

	bool operator==(const State& a, const State& b) {
		// sort m_basis so that the order does not matter here
		return a.m_basis == b.m_basis;
	}

	std::size_t hash_value(const State& set) {
		// sort m_basis so that the order does not matter here
		return boost::hash_range(set.m_basis.begin(), set.m_basis.end());
	}

}
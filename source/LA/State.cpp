#include "State.h"
#include "Grammar.h"

#include <cassert>
#include <algorithm>

namespace pitaya {

	State::State()
		: m_id {order()}, m_basis {}, m_closure {},
		m_is_final {false}, m_token_type {}, m_transitions {} {}

	State::State(State&& from) noexcept
		: m_id {order()}, m_is_final {from.m_is_final},
		m_basis {std::move(from.m_basis)},
		m_closure {std::move(from.m_closure)},
		m_transitions {std::move(from.m_transitions)} {
		// ensure 'from' is empty after move
		from.reset();
	}

	State::ID State::id() const {
		return m_id;
	}

	void State::add_symbol(const Symbol& s) {
		m_basis.push_back(s.id());
	}

	void State::compute_closure(Grammar& grammar) {
		// copy basis into closure
		m_closure.resize(grammar.symbol_count());
		for (auto& i : m_basis) {
			m_closure.add(grammar.get_symbol(i));
		}

		bool not_fin = false;
		do {
			not_fin = false;
			// for every nonterminal in the closure
			for (auto sid = grammar.terminal_count(); sid < grammar.symbol_count(); sid++) {
				if (!m_closure[sid]) continue;
				// for each production with symbol of sid as its lhs
				auto range = grammar.productions_by_lhs(sid);
				for (auto pid = range.first; pid <= range.second; pid++) {
					auto& p = grammar.get_production(pid);
					assert(p.rhs_count() <= 2);		// assert g is a regular grammar
					if (p.rhs_count() == 1) {
						auto& rhs = p[1];
						if (rhs.type() == SymbolType::NONTERMINAL) {
							// ε-transition
							not_fin = m_closure.add(p[1]);
						}
					}
					else if (p.rhs_count() == 0) {
						// this is a final state
						m_closure.add(grammar.endmark());
						m_is_final = true;
					}
				}
			}
		} while (not_fin);
	}

	bool& State::is_final() const {
		return m_is_final;
	}

	SymbolID& State::token_type() const {
		return m_token_type;
	}

	void State::add_transition(SymbolID symbol, State::ID state) const {
		auto& res = m_transitions.emplace(symbol, state);
		assert(res.second);		// DFA assertion
	}

	bool State::transit(const Symbol& symbol, ID& to) const {
		auto& find = m_transitions.find(symbol.id());
		if (find != m_transitions.end()) {
			to = find->second;
			return true;
		}
		return false;
	}

	void State::sort() {
		std::sort(m_basis.begin(), m_basis.end());
	}

	void State::reset() {
		m_basis.clear();
		m_closure.clear();
		m_transitions.clear();
		m_is_final = false;
	}

	const SymbolSet& State::closure() const {
		return m_closure;
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
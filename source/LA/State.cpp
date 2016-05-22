#include "State.h"
#include "Grammar.h"

#include <cassert>
#include <algorithm>

#include <boost\functional\hash\hash.hpp>

namespace pitaya {

	State::State()
		: m_id {order()}, m_basis {}, m_closure {},
		m_is_final {false}, m_token_index {}, m_transitions {} {}

	State::State(State&& from) noexcept
		: m_id {order()}, m_is_final {from.m_is_final}, m_token_index {from.m_token_index},
		m_basis {std::move(from.m_basis)},
		m_closure {std::move(from.m_closure)},
		m_transitions {std::move(from.m_transitions)} {
		// ensure 'from' is empty after move
		from.reset();
	}

	State::ID State::id() const {
		return m_id;
	}

	void State::add_base(const Production& p, BasicItem::Dot dot) {
		m_basis.emplace_back(p.id(), dot);
	}

	void State::compute_closure(Grammar& grammar) {
		// copy basis into closure
		for (auto& i : m_basis) {
			m_closure.emplace(i.production_id(), i.dot());
		}
		bool not_fin = false;
		do {
			not_fin = false;
			for (auto& item : m_closure) {
				auto& production = grammar.get_production(item.production_id());
				auto dot = item.dot();
				if (dot >= production.rhs_count()) {
					// this is a final state
					m_is_final = true;
					continue;
				}
				auto& symbol = production[dot + 1];				// symbol after dot
				if (symbol.type() == SymbolType::TERMINAL) {
					// 'symbol' is a terminal, cannot contribute any more items
					continue;
				}
				// for each production with 'symbol' as its lhs
				auto range = grammar.productions_by_lhs(symbol);
				for (auto pid = range.first; pid <= range.second; pid++) {
					auto& p = grammar.get_production(pid);
					auto& res = m_closure.emplace(p.id());
					not_fin = res.second;
				}
				if (not_fin) break;
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
		assert(res.second);		// DFA assertion
	}

	bool State::transit(const Symbol& symbol, ID& to) const {
		auto& find = m_transitions.find(symbol.rank);
		if (find != m_transitions.end()) {
			to = find->second;
			return true;
		}
		return false;
	}

	void State::sort() {
		std::sort(m_basis.begin(), m_basis.end(), [](auto& a, auto& b) {
			if (a.production_id() != b.production_id()) {
				return a.production_id() < b.production_id();
			}
			else {
				return a.dot() < b.dot();
			}
		});
	}

	void State::reset() {
		m_basis.clear();
		m_closure.clear();
		m_transitions.clear();
		m_is_final = false;
	}

	const std::unordered_set<BasicItem, boost::hash<BasicItem>>& State::closure() const {
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
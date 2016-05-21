#include "ItemSet.h"
#include "Grammar.h"
#include "ItemSetBuilder.h"

#include <algorithm>
#include <cassert>

#include <boost\functional\hash\hash.hpp>

namespace pitaya {

	ItemSet::ItemSet()
		: m_id {order()}, m_kernels {}, m_closure {}, m_actions {} {}

	ItemSet::ItemSet(ItemSet&& from) noexcept
		: m_id {order()},
		m_kernels {std::move(from.m_kernels)},
		m_closure {std::move(from.m_closure)},
		m_actions {std::move(from.m_actions)} {
		// ensure 'from' is empty after move
		from.reset();
	}

	StateID ItemSet::id() const {
		return m_id;
	}

	Item& ItemSet::add_kernel(const Production& p, Item::Dot d) {
		m_kernels.emplace_back(p.id(), d);
		return m_kernels.back();
	}

	const Item& ItemSet::get_kernel(std::size_t pos) const {
		return m_kernels[pos];
	}

	std::size_t ItemSet::kernel_count() const {
		return m_kernels.size();
	}

	void ItemSet::compute_closure(Grammar& grammar, ItemSetBuilder& builder) {
		// copy kernels into closure
		for (auto& i : m_kernels) {
			m_closure.emplace(i);
		}

		bool not_fin = false;
		do {
			not_fin = false;
			for (auto& item : m_closure) {
				if (item.complete) continue;
				auto& production = grammar.get_production(item.production_id());
				auto dot = item.dot();
				if (dot >= production.rhs_count()) {
					// skip item whose dot is at right end
					item.complete = true;
					continue;
				}
				auto& symbol = production[dot + 1];				// symbol after dot
				if (symbol.type() == SymbolType::TERMINAL) {
					// 'symbol' is a terminal, cannot contribute any more items
					item.complete = true;
					continue;
				}
				// for each production with 'symbol' as its lhs
				auto range = grammar.productions_by_lhs(symbol);
				for (auto pid = range.first; pid <= range.second; pid++) {
					auto& p = grammar.get_production(pid);
					auto& res = m_closure.emplace(p.id());
					not_fin = res.second;
					// update lookaheads generated spontaneously
					if (res.second) {
						res.first->lookaheads().resize(grammar.symbol_count());
					}
					// A -> α.Bβ, a
					// B -> γ, b where b ∈ first(βa)
					std::size_t i = dot + 1;
					for (i; i < production.rhs_count(); i++) {
						auto& rhs = production[i + 1];
						if (rhs.type() == SymbolType::TERMINAL) {
							res.first->lookaheads().add(rhs);
							break;
						}
						else {
							res.first->lookaheads().union_with(rhs.first_set());
							if (!rhs.lambda()) break;
						}
					}
					// if β is empty, add forward propagation link
					if (i == production.rhs_count()) {
						auto& new_node = builder.new_link();
						new_node->next = item.forward_plink();
						item.forward_plink() = new_node;
						new_node->item = &(*res.first);
					}
				}
				item.complete = true;
				if (not_fin) break;
			}
		} while (not_fin);
	}

	Action& ItemSet::add_action(const Symbol& symbol, ActionType type, std::size_t value) const {
		auto& res = m_actions.emplace(symbol.index(), Action {type, value});
		Action& act = res.first->second;
		if (!res.second) {
			// a conflict has occurred
			if (type == ActionType::SHIFT) {
				if (act.type == ActionType::SHIFT) {
					act.type = ActionType::SSCONFLICT;
				}
			}
			else if (type == ActionType::REDUCE) {
				if (act.type == ActionType::SHIFT) {
					act.type = ActionType::SRCONFLICT;
				}
				else if (act.type == ActionType::REDUCE) {
					act.type = ActionType::RRCONFLICT;
				}
			}
		}
		return act;
	}

	Action ItemSet::evaluate(const Symbol& symbol) const {
		auto& find = m_actions.find(symbol.index());
		if (find != m_actions.end()) {
			return find->second;
		}
		return Action {ActionType::ERROR, 0};
	}

	void ItemSet::sort() {
		std::sort(m_kernels.begin(), m_kernels.end(), [](auto& a, auto& b) {
			if (a.production_id() != b.production_id()) {
				return a.production_id() < b.production_id();
			}
			else {
				return a.dot() < b.dot();
			}
		});
	}

	void ItemSet::reset() {
		m_kernels.clear();
		m_closure.clear();
		m_actions.clear();
	}

	const std::unordered_set<Item, boost::hash<Item>>& ItemSet::closure() const {
		return m_closure;
	}

	bool operator==(const ItemSet& a, const ItemSet& b) {
		// sort m_kernels so that the order does not matter here
		return a.m_kernels == b.m_kernels;
	}

	std::size_t hash_value(const ItemSet& set) {
		// sort m_kernels so that the order does not matter here
		return boost::hash_range(set.m_kernels.begin(), set.m_kernels.end());
	}

}
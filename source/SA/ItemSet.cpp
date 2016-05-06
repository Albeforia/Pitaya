#include "ItemSet.h"
#include "Grammar.h"

#include <algorithm>

#include <boost\functional\hash\hash.hpp>

namespace pitaya {

	ItemSet::ItemSet()
		: m_kernels {}, m_closure {} {}

	ItemSet::ItemSet(ItemSet&& from) noexcept
		: m_kernels {std::move(from.m_kernels)}, m_closure {std::move(from.m_closure)} {
		// ensure 'from' is empty after move
		from.reset();
	}

	Item& ItemSet::add_kernel(const Production& p, Item::Dot d) {
		m_kernels.emplace_back(p.id(), d);
		return m_kernels.back();
	}

	void ItemSet::compute_closure(Grammar& g) {
		// copy kernels into closure
		for (auto& i : m_kernels) {
			m_closure.emplace(i);
		}

		bool not_fin = false;
		do {
			not_fin = false;
			for (auto& item : m_closure) {
				auto& production = g.get_production(item.production_id());
				auto dot = item.dot();
				if (dot >= production.rhs_count()) continue;	// skip kernel whose dot is at right end
				auto& symbol = production[dot + 1];				// symbol after dot
				if (symbol.type() == SymbolType::TERMINAL) continue;
				// for each production with 'symbol' as its lhs
				auto range = g.productions_by_lhs(symbol.id());
				for (auto pid = range.first; pid <= range.second; pid++) {
					auto& p = g.get_production(pid);
					auto& res = m_closure.emplace(p.id());
					not_fin = res.second;
					if (res.second) {
						// update lookaheads
						res.first->lookaheads().resize(g.terminal_count());
						// A -> α.Bβ, a
						// B -> γ, b where b ∈ first(βa)
						std::size_t i;
						for (i = dot + 1; i < production.rhs_count(); i++) {
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
							//
						}
					}
				}
				if (not_fin) break;
			}
		} while (not_fin);
	}

	void ItemSet::sort() {
		std::sort(m_kernels.begin(), m_kernels.end(), [](auto a, auto b) {
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
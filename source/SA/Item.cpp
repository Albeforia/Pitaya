#include "Item.h"

#include <iostream>

#include <boost\functional\hash\hash.hpp>

namespace pitaya {

	Item::Item(ProductionID pid, Dot dot)
		: m_production {pid}, m_dot {dot}, m_lookaheads {}, complete {} {}

	ProductionID Item::production_id() const {
		return m_production;
	}

	Item::Dot Item::dot() const {
		return m_dot;
	}

	bool Item::is_kernel() const {
		// assume initial item base on the first production
		return m_production == 0 && m_dot == 0 || m_dot != 0;
	}

	SymbolSet& Item::lookaheads() const {
		return m_lookaheads;
	}

	bool operator==(const Item& a, const Item& b) {
		return a.m_production == b.m_production && a.m_dot == b.m_dot;
	}

	std::size_t hash_value(const Item& item) {
		std::size_t seed = 0;
		boost::hash_combine(seed, item.m_production);
		boost::hash_combine(seed, item.m_dot);
		return seed;
	}

}
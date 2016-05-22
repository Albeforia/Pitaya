#include "BasicItem.h"

#include <boost\functional\hash\hash.hpp>

namespace pitaya {

	BasicItem::BasicItem(ProductionID pid, Dot dot)
		: m_production {pid}, m_dot {dot} {}

	ProductionID BasicItem::production_id() const {
		return m_production;
	}

	BasicItem::Dot BasicItem::dot() const {
		return m_dot;
	}

	bool operator==(const BasicItem& a, const BasicItem& b) {
		return a.m_production == b.m_production && a.m_dot == b.m_dot;
	}

	std::size_t hash_value(const BasicItem& BasicItem) {
		std::size_t seed = 0;
		boost::hash_combine(seed, BasicItem.m_production);
		boost::hash_combine(seed, BasicItem.m_dot);
		return seed;
	}

}
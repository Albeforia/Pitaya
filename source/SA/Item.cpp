#include "Item.h"

#include <boost\functional\hash\hash.hpp>

namespace pitaya {

	Item::Item(ProductionID pid, Dot dot)
		: m_production {pid}, m_dot {dot}, m_lookaheads {}, complete {},
		m_forward_plink {}, m_backward_plink {} {}

	Item::Item(Item&& from) noexcept
		: m_production {from.m_production}, m_dot {from.m_dot},
		m_lookaheads {std::move(from.m_lookaheads)}, complete {from.complete},
		m_forward_plink {from.m_forward_plink}, m_backward_plink {from.m_backward_plink} {
		from.m_forward_plink = nullptr;
		from.m_backward_plink = nullptr;
	}

	Item& Item::operator=(Item&& from) noexcept {
		m_production = from.m_production;
		m_dot = from.m_dot;
		m_lookaheads = std::move(from.m_lookaheads);
		complete = from.complete;
		m_forward_plink = from.m_forward_plink;
		m_backward_plink = from.m_backward_plink;
		from.m_forward_plink = nullptr;
		from.m_backward_plink = nullptr;
		return *this;
	}

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

	PLinkNode*& Item::forward_plink() const {
		return m_forward_plink;
	}

	PLinkNode*& Item::backward_plink() const {
		return m_backward_plink;
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
#include "Item.h"

namespace pitaya {

	Item::Item(ProductionID pid, Dot dot)
		: BasicItem(pid, dot),
		m_lookaheads {}, complete {false},
		m_forward_plink {}, m_backward_plink {} {}

	Item::Item(Item&& from) noexcept
		: BasicItem(from.m_production, from.m_dot),
		m_lookaheads {std::move(from.m_lookaheads)}, complete {from.complete},
		m_forward_plink {from.m_forward_plink}, m_backward_plink {from.m_backward_plink} {
		from.m_forward_plink = nullptr;
		from.m_backward_plink = nullptr;
	}

	Item& Item::operator=(Item&& from) noexcept {
		m_production = from.m_production;
		m_dot = from.m_dot;
		complete = from.complete;
		m_lookaheads = std::move(from.m_lookaheads);
		m_forward_plink = from.m_forward_plink;
		m_backward_plink = from.m_backward_plink;
		from.m_forward_plink = nullptr;
		from.m_backward_plink = nullptr;
		return *this;
	}

	bool Item::is_kernel() const {
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

}
#include "Production.h"

namespace pitaya {

	Production::Production()
		: m_lhs {}, m_rhs {} {}

	void Production::set_lhs(SharedSymbol lhs) {
		// move to avoid changing ref-count
		m_lhs = std::move(lhs);
	}

	std::vector<SharedSymbol>& Production::rhs() {
		return m_rhs;
	}

	Symbol& Production::operator[](std::size_t pos) const {
		if (pos == 0) {
			return *m_lhs;
		}
		return *m_rhs[pos - 1];
	}

	std::size_t Production::num_rhs() const {
		return m_rhs.size();
	}

}
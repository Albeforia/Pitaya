#include "Production.h"

namespace pitaya {

	Production::Production(ProductionID id, SharedSymbol lhs)
		: id {id}, m_lhs {std::move(lhs)}, m_rhs {} {}

	std::vector<SharedSymbol>& Production::rhs() {
		return m_rhs;
	}

	Symbol& Production::operator[](std::size_t pos) const {
		if (pos == 0) {
			return *m_lhs;
		}
		return *m_rhs[pos - 1];
	}

	std::size_t Production::rhs_count() const {
		return m_rhs.size();
	}

}
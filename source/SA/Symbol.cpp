#include "Symbol.h"

#include <cctype>

namespace pitaya {

	Symbol::Symbol(SymbolName name)
		: m_name {name}, m_lambda {false} {
		m_type = std::isupper(*m_name) ? SymbolType::TERMINAL : SymbolType::NONTERMINAL;
	}

	Symbol::SymbolName Symbol::name() const {
		return m_name;
	}

	std::size_t& Symbol::id() {
		return m_id;
	}

	SymbolType Symbol::type() const {
		return m_type;
	}

	bool& Symbol::lambda() {
		return m_lambda;
	}

}
#include "Symbol.h"

namespace pitaya {

	Symbol::Symbol(SymbolName name)
		: m_name {name}, m_type {SymbolType::UNDEFINED}, m_associativity {Associativity::UNDEFINED},
		m_lambda {false}, m_first_set {} {}

	Symbol::SymbolName Symbol::name() const {
		return m_name;
	}

	SymbolID& Symbol::id() {
		return m_id;
	}

	const SymbolID& Symbol::id() const {
		return m_id;
	}

	SymbolType& Symbol::type() {
		return m_type;
	}

	Associativity& Symbol::associativity() {
		return m_associativity;
	}

	bool& Symbol::lambda() {
		return m_lambda;
	}

	SymbolSet& Symbol::first_set() {
		return m_first_set;
	}

	bool operator==(const Symbol& a, const Symbol& b) {
		return a.m_id == b.m_id;
	}

	std::ostream& operator<<(std::ostream& os, const Symbol& s) {
		return os << s.m_name;
	}

}
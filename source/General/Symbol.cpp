#include "Symbol.h"

namespace pitaya {

	Symbol::Symbol(SymbolName name, SymbolID id)
		: m_name {name}, m_id {id}, m_type {SymbolType::UNDEFINED},
		m_associativity {Associativity::UNDEFINED}, m_precedence {-1},
		m_lambda {false}, m_first_set {} {}

	Symbol::SymbolName Symbol::name() const {
		return m_name;
	}

	SymbolID Symbol::id() const {
		return m_id;
	}

	SymbolType Symbol::type() const {
		return m_type;
	}

	Associativity Symbol::associativity() const {
		return m_associativity;
	}

	int Symbol::precedence() const {
		return m_precedence;
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

	bool operator!=(const Symbol& a, const Symbol& b) {
		return !(a.m_id == b.m_id);
	}

	std::ostream& operator<<(std::ostream& os, const Symbol& s) {
		return os << s.m_name;
	}

}
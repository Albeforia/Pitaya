#include "Symbol.h"

namespace pitaya {

	Symbol::Symbol(SymbolName name, Rank rank)
		: rank {rank}, m_name {name}, m_type {SymbolType::UNDEFINED},
		m_associativity {Associativity::UNDEFINED}, m_precedence {-1},
		m_is_token {false}, m_lambda {false}, m_first_set {},
		m_shared_terminal {nullptr} {}

	Symbol::SymbolName Symbol::name() const {
		return m_name;
	}

	std::size_t Symbol::index() const {
		return m_index;
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

	bool& Symbol::is_token() {
		return m_is_token;
	}

	bool& Symbol::lambda() {
		return m_lambda;
	}

	SymbolSet& Symbol::first_set() {
		return m_first_set;
	}

	Symbol& Symbol::shared_terminal() {
		return *m_shared_terminal;
	}

	bool operator==(const Symbol& a, const Symbol& b) {
		return a.rank == b.rank;
	}

	bool operator!=(const Symbol& a, const Symbol& b) {
		return !(a == b);
	}

	std::ostream& operator<<(std::ostream& os, const Symbol& s) {
		return os << s.m_name;
	}

}
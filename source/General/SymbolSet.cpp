#include "SymbolSet.h"
#include "Symbol.h"

#include <cassert>

namespace pitaya {

	SymbolSet::SymbolSet()
		: m_set {} {}

	std::size_t SymbolSet::size() const {
		return m_set.size();
	}

	void SymbolSet::resize(std::size_t n) {
		m_set.resize(n);
	}

	bool SymbolSet::add(const Symbol& s) {
		auto id = s.id();
		assert(id < size());
		if (m_set[id]) return false;
		return m_set[id] = true;
	}

	bool SymbolSet::union_with(const SymbolSet& s) {
		assert(size() == s.size());
		bool changed = false;
		for (std::size_t i = 0; i < size(); i++) {
			if (!s.m_set[i]) continue;
			if (!m_set[i]) {
				m_set[i] = true;
				changed = true;
			}
		}
		return changed;
	}

	bool SymbolSet::operator[](std::size_t id) const {
		assert(id < size());
		return m_set[id];
	}

	void SymbolSet::clear() {
		m_set.clear();
	}

}
#include "ItemSetBuilder.h"

#include <iostream>

namespace pitaya {

	ItemSetBuilder::ItemSetBuilder(std::shared_ptr<Grammar>& grammar)
		: m_grammar {grammar},
		m_item_sets {}, m_curr_item_set {} {}

	void ItemSetBuilder::build() {
		// initial item set
		m_curr_item_set.reset();
		// assume the first production is the augmented one
		// it's the only kernel for initial state
		auto& add = m_curr_item_set.add_kernel(m_grammar->get_production(0));
		// add '$' to the kernel's lookaheads
		add.lookaheads().resize(m_grammar->terminal_count());
		add.lookaheads().add(m_grammar->endmark());
		build_item_set();
	}

	void ItemSetBuilder::build_item_set() {
		m_curr_item_set.sort();		// sort kernels for hashing and comparing
		auto& find = m_item_sets.find(m_curr_item_set);
		if (find != m_item_sets.end()) {
			// a state with the same kernels already exists
		}
		else {
			// compute closure before moving
			m_curr_item_set.compute_closure(*m_grammar);
			// 'move' the currently building set into a new set
			auto& new_item = m_item_sets.emplace(std::move(m_curr_item_set)).first;
			// sort closure?
			//...
			build_successor(*new_item);
		}
	}

	void ItemSetBuilder::build_successor(const ItemSet& set) {
		for (auto& item : set.closure()) {
			if (item.complete) continue;
			auto& production = m_grammar->get_production(item.production_id());
			// skip item whose dot is at right end
			auto dot = item.dot();
			if (dot >= production.rhs_count()) {
				item.complete = true;
				continue;
			}
			auto& symbol = production[dot + 1];		// symbol after dot
			// for each item which has 'symbol' after its dot
			for (auto& item2 : set.closure()) {
				if (item2.complete) continue;
				auto& production2 = m_grammar->get_production(item2.production_id());
				auto dot2 = item2.dot();
				if (dot2 >= production2.rhs_count()) continue;	// no successor
				auto& symbol2 = production2[dot2 + 1];			// symbol after dot
				if (symbol == symbol2) {
					// each item becomes complete after contibuting a successor
					item2.complete = true;
					m_curr_item_set.add_kernel(production2, dot2 + 1);
					// add backward propagation link
					//...
				}
			}
			// build set from new kernels
			build_item_set();
		}
	}

	void ItemSetBuilder::print_all() const {
		for (auto& set : m_item_sets) {
			for (auto& item : set.closure()) {
				auto& p = m_grammar->get_production(item.production_id());
				std::cout << p[0] << "->";
				std::size_t i;
				for (i = 0; i < p.rhs_count(); i++) {
					if (item.dot() == i) {
						std::cout << ".";
					}
					std::cout << p[i + 1] << " ";
				}
				if (item.dot() == i) {
					std::cout << ".";
				}
				std::cout << "|";
				for (SymbolID i = 0; i < item.lookaheads().size(); i++) {
					if (item.lookaheads()[i]) {
						std::cout << m_grammar->get_symbol(i) << " ";
					}
				}
				std::cout << std::endl;
			}
			std::cout << std::endl;
		}
	}

}
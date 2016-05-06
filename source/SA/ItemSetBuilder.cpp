#include "ItemSetBuilder.h"

#include <cassert>
#include <iostream>

namespace pitaya {

	ItemSetBuilder::ItemSetBuilder(std::shared_ptr<Grammar>& grammar)
		: m_grammar {grammar}, m_item_sets {}, m_curr_item_set {} {}

	void ItemSetBuilder::build() {
		// initial item-set
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
			// a set with the same kernels already exists
			// copy all the backward propagation links
			for (std::size_t i = 0; i < m_curr_item_set.kernel_count(); i++) {
				auto& from_item = m_curr_item_set.get_kernel(i);
				auto& res = find->closure().find(from_item);
				// the closure must have been computed
				assert(res != find->closure().end());
				auto from = from_item.backward_plink();
				auto* to = &(res->backward_plink());
				PLinkNode* next;
				while (from != nullptr) {
					next = from->next;
					from->next = *to;
					*to = from;
					from = next;
				}
				from_item.backward_plink() = nullptr;
			}
			m_curr_item_set.reset();
		}
		else {
			// compute closure before moving
			m_curr_item_set.compute_closure(*m_grammar);
			// 'move' the currently building set into a new set
			auto& new_item = m_item_sets.emplace(std::move(m_curr_item_set)).first;
			// compute successor
			build_successor(*new_item);
		}
	}

	void ItemSetBuilder::build_successor(const ItemSet& set) {
		// reset
		for (auto& item : set.closure()) item.complete = false;

		for (auto& item : set.closure()) {
			if (item.complete) continue;
			auto& production = m_grammar->get_production(item.production_id());
			auto dot = item.dot();
			// skip item whose dot is at right end
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
				if (dot2 >= production2.rhs_count()) {
					// 'item2' has no successor
					item2.complete = true;
					continue;
				}
				auto& symbol2 = production2[dot2 + 1];			// symbol after dot
				if (symbol == symbol2) {
					// each item becomes complete after contibuting a successor
					item2.complete = true;
					auto& add = m_curr_item_set.add_kernel(production2, dot2 + 1);
					// add backward propagation link
					auto new_node = new PLinkNode {};
					new_node->next = add.backward_plink();
					add.backward_plink() = new_node;
					new_node->item = &item2;
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
				std::size_t i = 0;
				for (i; i < p.rhs_count(); i++) {
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
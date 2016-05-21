#include "ItemSetBuilder.h"

#include <cassert>
#include <iostream>

namespace pitaya {

	ItemSetBuilder::ItemSetBuilder(Grammar& grammar)
		: m_grammar {grammar}, m_item_sets {}, m_curr_item_set {},
		m_plinks {} {}

	ItemSetBuilder::~ItemSetBuilder() {
		for (auto& p : m_plinks) {
			delete p;
		}
	}

	void ItemSetBuilder::build() {
		compute_first_sets();
		// initial item-set
		m_curr_item_set.reset();
		// assume the first production is the augmented one
		// it's the only kernel for initial state
		auto& add = m_curr_item_set.add_kernel(m_grammar.get_production(0));
		// add '$' to the kernel's lookaheads
		add.lookaheads().resize(m_grammar.symbol_count());
		add.lookaheads().add(m_grammar.endmark());
		build_item_set();
		fill_lookaheads();
		fill_actions();
	}

	void ItemSetBuilder::compute_first_sets() {
		for (auto it = m_grammar.nonterminal_begin(); it != m_grammar.nonterminal_end(); it++) {
			(*it)->first_set().resize(m_grammar.symbol_count());
		}

		bool not_fin = false;

		// compute all lambdas
		do {
			not_fin = false;
			for (ProductionID pid = 0; pid < m_grammar.production_count(); pid++) {
				auto& p = m_grammar.get_production(pid);
				if (p[0].lambda()) continue;
				std::size_t i = 0;
				for (i; i < p.rhs_count(); i++) {
					auto& rhs = p[i + 1];
					assert(rhs.type() == SymbolType::NONTERMINAL || !rhs.lambda());
					if (!rhs.lambda()) break;		// lhs is lambda <=> all rhs are lambda
				}
				if (i == p.rhs_count()) {				// including zero rhs
					p[0].lambda() = true;
					not_fin = true;					// find a new lambda, continue computing
				}
			}
		} while (not_fin);

		// compute all first sets
		do {
			not_fin = false;
			for (ProductionID pid = 0; pid < m_grammar.production_count(); pid++) {
				auto& p = m_grammar.get_production(pid);
				auto& lhs = p[0];
				for (std::size_t i = 0; i < p.rhs_count(); i++) {
					auto& rhs = p[i + 1];
					if (rhs.type() == SymbolType::TERMINAL) {
						not_fin = lhs.first_set().add(rhs);
						break;		// encounter a terminal, add to first set and stop
					}
					else if (lhs == rhs) {
						if (!lhs.lambda()) break;	// recurrence happened, should compute in another production
					}
					else {
						not_fin = lhs.first_set().union_with(rhs.first_set());
						if (!rhs.lambda()) break;	// stop if a rhs cannot generate empty string
					}
				}
			}
		} while (not_fin);
	}

	const ItemSet& ItemSetBuilder::build_item_set() {
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
			return *find;
		}
		// a new item-set
		// compute closure before moving
		m_curr_item_set.compute_closure(m_grammar, *this);
		// 'move' the currently building set into the new set
		auto& new_item = m_item_sets.emplace(std::move(m_curr_item_set)).first;
		// compute successors
		build_successors(*new_item);
		return *new_item;
	}

	void ItemSetBuilder::build_successors(const ItemSet& set) {
		// reset
		for (auto& item : set.closure()) item.complete = false;

		for (auto& item : set.closure()) {
			if (item.complete) continue;
			auto& production = m_grammar.get_production(item.production_id());
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
				auto& production2 = m_grammar.get_production(item2.production_id());
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
					add.lookaheads().resize(m_grammar.symbol_count());
					// add backward propagation link
					auto& new_node = new_link();
					new_node->next = add.backward_plink();
					add.backward_plink() = new_node;
					new_node->item = &item2;
				}
			}
			// build set from new kernels
			auto& new_set = build_item_set();
			auto& act = set.add_action(symbol, ActionType::SHIFT, new_set.id());
			assert(act.type != ActionType::SSCONFLICT);
		}
	}

	void ItemSetBuilder::fill_lookaheads() {
		// convert all backward links into forward links
		for (auto& set : m_item_sets) {
			for (auto& item : set.closure()) {
				for (auto bpl = item.backward_plink(); bpl != nullptr; bpl = bpl->next) {
					auto& fpl = bpl->item->forward_plink();
					auto& new_node = new_link();
					new_node->next = fpl;
					fpl = new_node;
					new_node->item = &item;
				}
			}
		}

		// reset
		for (auto& set : m_item_sets) {
			for (auto& item : set.closure()) {
				item.complete = false;
			}
		}

		bool not_fin = false;
		do {
			not_fin = false;
			for (auto& set : m_item_sets) {
				for (auto& item : set.closure()) {
					if (item.complete) continue;
					for (auto fpl = item.forward_plink(); fpl != nullptr; fpl = fpl->next) {
						bool change = fpl->item->lookaheads().union_with(item.lookaheads());
						if (change) {
							fpl->item->complete = false;
							not_fin = true;
						}
					}
					item.complete = true;
				}
			}
		} while (not_fin);
	}

	void ItemSetBuilder::fill_actions() {
		for (auto& set : m_item_sets) {
			for (auto& item : set.closure()) {
				auto& production = m_grammar.get_production(item.production_id());
				// for every production whose dot is at right end
				if (item.dot() == production.rhs_count()) {
					for (auto it = m_grammar.terminal_begin(); it != m_grammar.terminal_end(); it++) {
						if (item.lookaheads()[**it]) {
							if (production.id() == 0) {
								set.add_action(**it, ActionType::ACCEPT, production.id());
							}
							else {
								set.add_action(**it, ActionType::REDUCE, production.id());
							}
						}
					}
				}
			}
		}
	}

	PLinkNode*& ItemSetBuilder::new_link() {
		m_plinks.push_back(new PLinkNode {});
		return m_plinks.back();
	}

	void ItemSetBuilder::print_all() const {
		for (auto& set : m_item_sets) {
			std::cout << "state " << set.id() << std::endl;
			for (auto& item : set.closure()) {
				auto& p = m_grammar.get_production(item.production_id());
				std::cout << "\t" << p[0] << "->";
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
				std::cout << "\t" << "|";
				for (auto it = m_grammar.terminal_begin(); it != m_grammar.terminal_end(); it++) {
					if (item.lookaheads()[**it]) {
						std::cout << **it << " ";
					}
				}
				std::cout << std::endl;
			}
			std::cout << std::endl;
		}
	}

}
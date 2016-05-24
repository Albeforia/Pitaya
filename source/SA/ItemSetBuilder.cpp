#include "ItemSetBuilder.h"

#include <cassert>
#include <fstream>
#include <iostream>
#include <iomanip>

namespace pitaya {

	ItemSetBuilder::ItemSetBuilder(Grammar& grammar)
		: m_grammar {grammar}, m_item_sets {}, m_curr_item_set {},
		m_plinks {}, m_conflict_count {} {}

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

		auto progress = 0;
		do {
			progress = 0;
			for (ProductionID pid = 0; pid < m_grammar.production_count(); pid++) {
				auto& p = m_grammar.get_production(pid);
				auto& lhs = p[0];
				for (std::size_t i = 0; i < p.rhs_count(); i++) {
					auto& rhs = p[i + 1];
					if (rhs.type() != SymbolType::NONTERMINAL) {
						if (lhs.first_set().add(rhs)) {
							progress++;
						}
						break;		// encounter a (multi)terminal, add to first set and stop
					}
					else if (lhs == rhs) {
						if (!lhs.lambda()) break;	// recurrence happened, should compute in another production
					}
					else {
						if (lhs.first_set().union_with(rhs.first_set())) {
							progress++;
						}
						if (!rhs.lambda()) break;	// stop if a rhs cannot generate empty string
					}
				}
			}
		} while (progress != 0);
	}

	const ItemSet& ItemSetBuilder::build_item_set() {
		m_curr_item_set.sort();		// sort kernels for hashing and comparing
		auto& find = m_item_sets.find(m_curr_item_set);
		if (find != m_item_sets.end()) {
			// a set with the same kernels already exists
			// copy all the backward propagation links
			for (std::size_t i = 0; i < m_curr_item_set.kernel_count(); i++) {
				auto& from_item = m_curr_item_set.get_kernel(i);
				auto& res = find->m_closure.find(from_item);
				// the closure must have been computed
				assert(res != find->m_closure.end());
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
		for (auto& item : set.m_closure) item.complete = false;

		for (auto& item : set.m_closure) {
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
			for (auto& item2 : set.m_closure) {
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
			if (symbol.type() == SymbolType::NONTERMINAL) {
				auto& act = set.add_action(symbol, ActionType::GOTO, new_set.id());
			}
			else {
				auto& act = set.add_action(symbol, ActionType::SHIFT, new_set.id());
				assert(act.type != ActionType::SSCONFLICT);
			}
		}
	}

	void ItemSetBuilder::fill_lookaheads() {
		// convert all backward links into forward links
		for (auto& set : m_item_sets) {
			for (auto& item : set.m_closure) {
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
			for (auto& item : set.m_closure) {
				item.complete = false;
			}
		}

		bool not_fin = false;
		do {
			not_fin = false;
			for (auto& set : m_item_sets) {
				for (auto& item : set.m_closure) {
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
		for (auto& state : m_item_sets) {
			m_sorted.emplace(state.m_id, &state);
			for (auto& item : state.m_closure) {
				auto& production = m_grammar.get_production(item.production_id());
				// for every production whose dot is at right end
				if (item.dot() == production.rhs_count()) {
					for (auto it = m_grammar.symbol_begin(); it != m_grammar.symbol_end(); it++) {
						if (item.lookaheads()[**it]) {
							if (production.id() == 0) {
								state.add_action(**it, ActionType::ACCEPT, production.id());
							}
							else {
								Action new_act {ActionType::REDUCE, production.id()};
								auto& act = state.add_action(**it, new_act.type, new_act.value);
								resolve_conflict(act, new_act, **it, state);
							}
						}
					}
				}
			}
		}
	}

	bool ItemSetBuilder::resolve_conflict(Action& origin, Action& conflict,
										  const Symbol& sym, const ItemSet& state) {
		static auto first_time = true;
		static auto mode = std::ios::trunc;
		if (first_time) {
			first_time = false;
		}
		else {
			mode = std::ios::app;
		}

		std::ofstream file;
		file.open("report\\conflicts", mode);
		// TODO use precedence and associativity to resolve conflicts
		if (origin.type == ActionType::SRCONFLICT) {
			m_conflict_count++;
			// simply discard SHIFT
			origin.type = ActionType::REDUCE;
			origin.value = conflict.value;
			if (file.is_open()) {
				file << "[ SRCONFLICT in " << state.m_id << " ]\n"
					<< '\t' << sym << '\n'
					<< '\t' << m_grammar.get_production(conflict.value) << '\n'
					<< "resolved to\t" << sym << "\n\n";
			}
			m_conflict_count--;
			return true;
		}
		else if (origin.type == ActionType::RRCONFLICT) {
			m_conflict_count++;
			// simply keep the origin
			origin.type = ActionType::REDUCE;
			if (file.is_open()) {
				file << "[ RRCONFLICT in " << state.m_id << " ]\n"
					<< '\t' << m_grammar.get_production(origin.value) << '\n'
					<< '\t' << m_grammar.get_production(conflict.value) << '\n'
					<< "resolved to\t" << m_grammar.get_production(origin.value) << "\n\n";
			}
			m_conflict_count--;
			return true;
		}
		file.close();
		return true;
	}

	const ItemSet& ItemSetBuilder::get_state(StateID id) const {
		return *m_sorted.at(id);
	}

	PLinkNode*& ItemSetBuilder::new_link() {
		m_plinks.push_back(new PLinkNode {});
		return m_plinks.back();
	}

	void ItemSetBuilder::print_all() const {
		std::ofstream file;
		file.open("report\\lalr_states", std::ios::trunc);
		if (file.is_open()) {
			std::string dot = "[>";
			for (auto& p : m_sorted) {
				auto& set = *p.second;
				file << "[state " << set.m_id << "]\n";
				///*
				for (auto& item : set.m_closure) {
					if (!item.is_kernel()) continue;
					auto& p = m_grammar.get_production(item.production_id());
					file << ">\t" << std::setw(20) << std::left << p[0] << "==>\t";
					for (std::size_t i = 0; i <= p.rhs_count(); i++) {
						if (item.dot() == i) {
							file << dot;
						}
						if (0 <= i && i < p.rhs_count()) {
							file << p[i + 1] << " ";
						}
					}
					file << "\n\t\t";
					for (auto it = m_grammar.symbol_begin(); it != m_grammar.symbol_end(); it++) {
						if (item.lookaheads()[**it]) {
							file << **it << "\t";
						}
					}
					file << "\n";
				}
				file << '\n';
				//*/
				for (auto& action : set.m_actions) {
					file << ">\t" << std::setw(30)
						<< std::left << m_grammar.get_symbol(action.first)
						<< std::right << action.second.type;
					if (action.second.type == ActionType::SHIFT
						|| action.second.type == ActionType::GOTO) {
						file << std::setw(10) << "[ state " << action.second.value << " ]";
					}
					else if (action.second.type == ActionType::REDUCE) {
						file << std::setw(10) << "[ " << m_grammar.get_production(action.second.value) << " ]";
					}
					file << '\n';
				}
				file << '\n';
			}
			file << "Total conflicts: " << m_conflict_count << '\n';
		}
		else {
			std::cout << "fail opening file!" << std::endl;
		}
		file.close();
	}

}
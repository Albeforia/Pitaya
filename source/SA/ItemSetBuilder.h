#pragma once

#include <unordered_set>
#include <map>

#include "Grammar.h"
#include "ItemSet.h"

namespace pitaya {

	//! ItemSetBuilder class.
	class ItemSetBuilder {

	public:

		//! Constructor.
		ItemSetBuilder(Grammar&);

		//! Destructor.
		~ItemSetBuilder();

		//! Build all item sets.
		void build();

		//! Get a state by id.
		const ItemSet& get_state(StateID) const;

		//! Get a new PLinkNode.
		PLinkNode*& new_link();

		/// @cond test
		void print_all() const;
		/// @endcond

	private:

		Grammar& m_grammar;		//!< The grammar this builder works on.

		std::unordered_set<ItemSet, boost::hash<ItemSet>> m_item_sets;	//!< All ItemSets.
		std::map<StateID, const ItemSet*> m_sorted;		//!< Sorted item-sets for quick access.
		ItemSet m_curr_item_set;		//!< The ItemSet being built currently.

		std::vector<PLinkNode*> m_plinks;		//!< Maintain all PLinkNodes.

		std::size_t m_conflict_count;			//!< Number of conflicts.

		//! Compute the first sets of every nonterminal.
		void compute_first_sets();

		//! Build or merge item-set according to m_curr_item_set.
		const ItemSet& build_item_set();

		//! Build all successors of an item-set.
		void build_successors(const ItemSet&);

		//! Compute all lookaheads.
		void fill_lookaheads();

		//! Generate all actions.
		void fill_actions();

	};

}
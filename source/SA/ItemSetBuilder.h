#pragma once

#include <unordered_set>

#include <boost\functional\hash\hash.hpp>

#include "Grammar.h"
#include "ItemSet.h"

namespace pitaya {

	//! ItemSetBuilder class.
	class ItemSetBuilder {

	public:

		//! Constructor.
		ItemSetBuilder(std::shared_ptr<Grammar>&);

		//! Destructor.
		~ItemSetBuilder();

		//! Build all item sets.
		void build();

		//! Get a new PLinkNode.
		PLinkNode*& new_link();

		/// @cond test
		void print_all() const;
		/// @endcond

	private:

		std::shared_ptr<Grammar> m_grammar;		//!< The grammar this builder works on.

		//! All ItemSets.
		std::unordered_set<ItemSet, boost::hash<ItemSet>> m_item_sets;
		//! The ItemSet being built currently.
		ItemSet m_curr_item_set;

		std::vector<PLinkNode*> m_plinks;		//! Maintain all PLinkNodes.

		//! Compute the first sets of every nonterminal.
		void compute_first_sets();

		//!
		const ItemSet& build_item_set();

		//!
		void build_successors(const ItemSet&);

		//!
		void fill_lookaheads();

		//!
		void fill_actions();

	};

}
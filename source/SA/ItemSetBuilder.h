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

		//! Build all item sets.
		void build();

		/// @cond test
		void print_all() const;
		/// @endcond

	private:

		std::shared_ptr<Grammar> m_grammar;		//!< The grammar this builder works on.

		//! All ItemSets.
		std::unordered_set<ItemSet, boost::hash<ItemSet>> m_item_sets;
		//! The ItemSet being built currently.
		ItemSet m_curr_item_set;

		//!
		void build_item_set();

		//!
		void build_successor(const ItemSet&);

	};

}
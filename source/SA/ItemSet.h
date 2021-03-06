#pragma once

#include "Item.h"
#include "Action.h"

#include <vector>
#include <unordered_set>
#include <unordered_map>

#include <boost\functional\hash\hash.hpp>

namespace pitaya {

	using StateID = std::size_t;

	class Production;
	class Grammar;

	/*!
		\ingroup SA
		ItemSet class.
	*/
	class ItemSet {

	public:

		friend class ItemSetBuilder;

		//! Constructor.
		ItemSet();

		//! Move constructor.
		ItemSet(ItemSet&&) noexcept;

		//! ID of the set(or state).
		StateID id() const;

		//! Add a kernel to the set.
		/*!
			\return The added kernel.
		*/
		Item& add_kernel(const Production&, Item::Dot = 0);

		//! Compute the closure of kernels.
		void compute_closure(Grammar&, ItemSetBuilder&);

		//! Sort the kernels.
		void sort();

		//! Add an action.
		/*!
			\return The added action(a conflict may have occurred).
		*/
		Action& add_action(const Symbol&, ActionType, std::size_t value) const;

		//! Evaluate the action against a symbol.
		Action evaluate(const Symbol&) const;

		//! Clear all items.
		void reset();

		//! Two ItemSets are equal iff they have the same kernels.
		friend bool operator==(const ItemSet&, const ItemSet&);

		//! Hash.
		friend std::size_t hash_value(const ItemSet&);

	private:

		//! All kernels in this set.
		std::vector<Item> m_kernels;
		//! Closure of this set.
		std::unordered_set<Item, boost::hash<Item>> m_closure;

		StateID m_id;		//!< ID of the set(or state).

		//! Actions of this state.
		mutable std::unordered_map<std::size_t, Action> m_actions;

		/// @cond
		//! Unique ID marking the first appearance of an item-set(or state).
		static std::size_t order() {
			static std::size_t interned_ = 0;
			return interned_++;
		}
		/// @endcond

	};

}
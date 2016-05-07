#pragma once

#include "Item.h"

#include <vector>
#include <unordered_set>

#include <boost\functional\hash\hash.hpp>

namespace pitaya {

	class Production;
	class Grammar;
	class ItemSetBuilder;

	//! ItemSet class.
	class ItemSet {

	public:

		//! Constructor.
		ItemSet();

		//! Move constructor.
		ItemSet(ItemSet&&) noexcept;

		//! Add a kernel to the set.
		/*!
			\return The added kernel.
		*/
		Item& add_kernel(const Production&, Item::Dot = 0);

		//! Get a kernel.
		const Item& get_kernel(std::size_t pos) const;

		//! Number of kernels.
		std::size_t kernel_count() const;

		//! Compute the closure of kernels.
		void compute_closure(Grammar&, ItemSetBuilder&);

		//! Sort the kernels.
		void sort();

		//! Clear all items.
		void reset();

		//! Two ItemSets are equal iff they have the same kernels.
		friend bool operator==(const ItemSet&, const ItemSet&);

		//! Hash.
		friend std::size_t hash_value(const ItemSet&);

	private:

		std::vector<Item> m_kernels;				//!< All kernels in this set.
		//! Closure of this set.
		std::unordered_set<Item, boost::hash<Item>> m_closure;

	public:

		//! Getter for m_closure.
		const decltype(m_closure)& closure() const;

	};

}
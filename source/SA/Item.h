#pragma once

#include "Production.h"

namespace pitaya {

	class Item;
	struct PLinkNode {
		const Item* item;
		PLinkNode* next;
		PLinkNode() : item {}, next {} {}
	};

	//! Item class.
	class Item {

	public:

		using Dot = std::size_t;

		//! Constructor.
		explicit Item(ProductionID, Dot = 0);

		//! Move constructor.
		Item(Item&&) noexcept;
		Item& operator=(Item&&) noexcept;

		//! Copy constructor.
		Item(const Item&) = default;
		Item& operator=(const Item&) = default;

		//! Destructor.
		~Item();

		//! The id of the production upon which the item is based.
		ProductionID production_id() const;

		//! The parse point.
		Dot dot() const;

		//! Whether the item is a kernel.
		bool is_kernel() const;

		//! Getter for m_lookaheads.
		SymbolSet& lookaheads() const;

		//! Getter for m_forward_plink.
		PLinkNode*& forward_plink() const;

		//! Getter for m_backward_plink.
		PLinkNode*& backward_plink() const;

		//! Equality.
		friend bool operator==(const Item&, const Item&);

		//! Hash.
		friend std::size_t hash_value(const Item&);

		/// @cond test
		mutable bool complete;		// used in item-set closure\successor computing
		/// @endcond

	private:

		//! The id of the production upon which the item is based.
		/*!
			Note item should not own a production, so we use id instead of ptr here.
		*/
		ProductionID m_production;
		Dot m_dot;					//!< The parse point.
		//! Lookaheads of the item.
		/*!
			Declare mutable because it will be updated after being added to set.
		*/
		mutable SymbolSet m_lookaheads;

		mutable PLinkNode* m_forward_plink;		//!< Forward propagation link.
		mutable PLinkNode* m_backward_plink;	//!< Backward propagation link.

	};

}
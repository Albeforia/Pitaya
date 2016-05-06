#pragma once

#include "Production.h"

namespace pitaya {

	//! Item class.
	class Item {

	public:

		using Dot = std::size_t;

		//! Constructor.
		Item(ProductionID, Dot = 0);

		//! The id of the production upon which the item is based.
		ProductionID production_id() const;

		//! The parse point.
		Dot dot() const;

		//! Whether the item is a kernel.
		bool is_kernel() const;

		//! Getter for m_lookaheads.
		SymbolSet& lookaheads() const;

		//! Equality.
		friend bool operator==(const Item&, const Item&);

		//! Hash.
		friend std::size_t hash_value(const Item&);

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

	public:

		/// @cond test
		mutable bool complete;		// used in item set successor computing
		/// @endcond

	};

}
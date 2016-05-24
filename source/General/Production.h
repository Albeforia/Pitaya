#pragma once

#include "Symbol.h"

#include <vector>

namespace pitaya {

	using ProductionID = std::size_t;

	//! Production class.
	class Production {

		friend class Grammar;

	public:

		//! Constructor.
		Production(Rank, SharedSymbol lhs);

		//! ID of the production.
		ProductionID id() const;

		//! Rank of the production.
		Rank rank() const;

		//! Number of rhs.
		std::size_t rhs_count() const;

		//! Index lhs and rhs of this production.
		/*!
			\param pos Return lhs when pos equals zero, otherwise return the 'pos'th rhs.
		*/
		Symbol& operator[](std::size_t pos) const;

		//! Output.
		friend std::ostream& operator<<(std::ostream&, const Production&);

	private:

		//! Unique number marking the first appearance of the production.
		Rank m_rank;

		ProductionID m_id;					//!< ID of the production.
		SharedSymbol m_lhs;					//!< Left-hand side of the production.
		std::vector<SharedSymbol> m_rhs;	//!< Right-hand side of the production.

	};

}
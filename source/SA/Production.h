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
		/*!
			\param id ID of the production.
			\param lhs Left-hand side of the production.
		*/
		Production(ProductionID id, SharedSymbol lhs);

		//! ID of the production.
		ProductionID id() const;

		//! Number of rhs.
		std::size_t rhs_count() const;

		//! Index lhs and rhs of this production.
		/*!
			\param pos Return lhs when pos equals zero, otherwise return the 'pos'th rhs.
		*/
		Symbol& operator[](std::size_t pos) const;

	private:

		ProductionID m_id;					//!< ID of the production.
		SharedSymbol m_lhs;					//!< Left-hand side of the production.
		std::vector<SharedSymbol> m_rhs;	//!< Right-hand side of the production.

	};

}
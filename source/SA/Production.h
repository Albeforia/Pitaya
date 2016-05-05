#pragma once

#include "Symbol.h"

#include <vector>

namespace pitaya {

	class Symbol;

	using ProductionID = std::size_t;

	//! Production class.
	class Production {

	public:

		//! Constructor.
		/*!
			\param id ID of the production.
			\param lhs Left-hand side of the production.
		*/
		Production(ProductionID id, SharedSymbol lhs);

		//! Getter for rhs.
		/*!
			Should only use during parsing grammar file.
		*/
		std::vector<SharedSymbol>& rhs();

		//! Index lhs and rhs of this production.
		/*!
			\param pos Return lhs when pos equals zero, otherwise return the 'pos'th rhs.
		*/
		Symbol& operator[](std::size_t pos) const;

		//! Number of rhs.
		std::size_t rhs_count() const;

		const ProductionID id;				//!< ID of the production.

	private:

		SharedSymbol m_lhs;					//!< Left-hand side of the production.
		std::vector<SharedSymbol> m_rhs;	//!< Right-hand side of the production.

	};

}
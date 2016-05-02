#pragma once

#include "Symbol.h"

#include <vector>

namespace pitaya {

	class Symbol;

	//! Production class.
	class Production {

	public:

		//! Default constructor.
		Production();

		//! Setter for lhs.
		/*!
			Should only use during parsing grammar file.
		*/
		void set_lhs(SharedSymbol);

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
		std::size_t num_rhs() const;

	private:

		SharedSymbol m_lhs;					//!< Left-hand side of the rule.
		std::vector<SharedSymbol> m_rhs;	//!< Right-hand side of the rule.

	};

}
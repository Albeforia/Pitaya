#pragma once

#include "Production.h"

namespace pitaya {

	//! BasicItem class.
	class BasicItem {

	public:

		using Dot = std::size_t;

		//! Constructor.
		BasicItem(ProductionID, Dot = 0);

		//! The id of the production upon which the item is based.
		ProductionID production_id() const;

		//! The parse point.
		Dot dot() const;

		//! Equality.
		friend bool operator==(const BasicItem&, const BasicItem&);

		//! Hash.
		friend std::size_t hash_value(const BasicItem&);

	protected:

		ProductionID m_production;	//!< The id of the production upon which the item is based.
		Dot m_dot;					//!< The parse point.

	};

}
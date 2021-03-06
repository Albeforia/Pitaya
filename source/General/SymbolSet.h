#pragma once

#include <boost\dynamic_bitset\dynamic_bitset.hpp>

namespace pitaya {

	class Symbol;

	/*!
		\ingroup General
		Set of symbols.
	*/
	class SymbolSet {

	public:

		//! Default constructor.
		SymbolSet();

		//! Size of the set.
		std::size_t size() const;

		//! Change set size.
		/*!
			\param n New size.
		*/
		void resize(std::size_t n);

		//! Add a symbol.
		/*!
			\return False if the symbol is already in the set.
		*/
		bool add(const Symbol&);

		//! Union with another set.
		/*!
			\return False if this set does not change after union.
		*/
		bool union_with(const SymbolSet&);

		//! Whether the symbol is in set.
		bool operator[](const Symbol&) const;

		//! Clear all symbols.
		void clear();

	private:

		boost::dynamic_bitset<> m_set;		//!< Underlying set representation.

	};

}
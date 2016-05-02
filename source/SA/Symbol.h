#pragma once

#include "SymbolSet.h"

#include <string>
#include <memory>
#include <unordered_map>
#include <iostream>

namespace pitaya {

	using SharedSymbol = std::shared_ptr<Symbol>;

	//! Symbol type.
	enum class SymbolType {
		TERMINAL,
		NONTERMINAL,
	};

	//! Symbol class.
	class Symbol : public std::enable_shared_from_this<Symbol> {

	public:

		using SymbolName = const char*;

		//! Name of this symbol.
		SymbolName name() const;

		//! ID of the symbol.
		std::size_t& id();
		const std::size_t& id() const;

		//! Type of this symbol.
		SymbolType type() const;

		//! Whether this symbol can generate an empty string.
		bool& lambda();

		//! First set of the symbol.
		SymbolSet& first_set();

		//! Output.
		friend std::ostream& operator<<(std::ostream&, const Symbol&);

		//! Use this factory function to 'create' symbols.
		/*!
			This function ensures a symbol is stored only once
			and a shared_ptr has been created before calling shared_from_this().
			\param params Any parameters can be used to construct a string.
			\return A shared_ptr of the symbol.
		*/
		template<typename... Ts>
		static typename std::enable_if<
			std::is_constructible<std::string, Ts...>::value, SharedSymbol
		>::type create(Ts&&... params) {
			SharedSymbol ps;		// empty
			auto& res = pool().emplace(std::forward<Ts>(params)..., std::move(ps));
			if (res.second) {
				auto& emplaced = res.first;
				emplaced->second.reset(new Symbol {emplaced->first.c_str()});
				emplaced->second->id() = order();
				return emplaced->second;
			}
			else {
				// no insertion
				return res.first->second->shared_from_this();
			}
		}

		/// @cond
		//! Symbol pool.
		/*!
			Guarantee static pool is initialised before accessing.
		*/
		static auto& pool() {
			static std::unordered_map<std::string, SharedSymbol> interned_;
			return interned_;
		}
		/// @endcond

	private:

		//! Constructor.
		/*!
			Use create() instead.
		*/
		Symbol(SymbolName name);

		SymbolName m_name;		//!< Name of the symbol.
		std::size_t m_id;		//!< ID of the symbol.
		SymbolType m_type;		//!< Type of the symbol.
		bool m_lambda;			//!< True if this symbol can generate an empty string.
		SymbolSet m_first_set;	//!< First set of the symbol.

		/// @cond
		//! Unique ID marking the first appearance of a symbol.
		static std::size_t order() {
			static std::size_t interned_ = 0;
			return interned_++;
		}
		/// @endcond

	};

}
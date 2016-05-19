#pragma once

#include "SymbolSet.h"

#include <string>
#include <memory>
#include <unordered_map>
#include <iostream>

namespace pitaya {

	using SharedSymbol = std::shared_ptr<Symbol>;
	using SymbolID = std::size_t;

	//! Symbol type.
	enum class SymbolType {
		UNDEFINED,
		TERMINAL,
		NONTERMINAL,
	};

	//! Symbol associativity.
	enum class Associativity {
		UNDEFINED,
		LEFT,
		RIGHT,
		NONE
	};

	//! Symbol class.
	class Symbol : public std::enable_shared_from_this<Symbol> {

		friend class Grammar;

	public:

		using SymbolName = const char*;

		//! Name of this symbol.
		SymbolName name() const;

		//! ID of this symbol.
		SymbolID id() const;

		//! Type of this symbol.
		SymbolType type() const;

		//! Associativity of this symbol.
		Associativity associativity() const;

		//! Precedence of this symbol.
		int precedence() const;

		//! Whether this symbol can generate an empty string.
		bool& lambda();

		//! First set of this symbol.
		SymbolSet& first_set();

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
				emplaced->second.reset(new Symbol {emplaced->first.c_str(), order()});
				return emplaced->second;
			}
			else {
				// no insertion
				return res.first->second->shared_from_this();
			}
		}

		//! Equality.
		friend bool operator==(const Symbol&, const Symbol&);

		//! Output.
		friend std::ostream& operator<<(std::ostream&, const Symbol&);

	private:

		//! Constructor.
		/*!
			Use create() instead.
		*/
		Symbol(SymbolName, SymbolID);

		SymbolID m_id;					//!< ID of the symbol.
		SymbolName m_name;				//!< Name of the symbol.
		SymbolType m_type;				//!< Type of the symbol.
		Associativity m_associativity;	//!< Associativity of the symbol.
		int m_precedence;				//!< Precedence of the symbol.
		bool m_lambda;					//!< True if the symbol can generate an empty string.
		SymbolSet m_first_set;			//!< First set of the symbol.

		/// @cond

		// guarantee the static pool is initialised before accessing
		static auto& pool() {
			static std::unordered_map<std::string, SharedSymbol> interned_;
			return interned_;
		}

		// unique ID marking the first appearance of a symbol
		static std::size_t order() {
			static std::size_t interned_ = 0;
			return interned_++;
		}

		/// @endcond

	};

}
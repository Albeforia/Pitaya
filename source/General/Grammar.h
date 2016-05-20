#pragma once

#include "Production.h"

#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/event.hpp>
#include <boost/statechart/custom_reaction.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/mpl/list.hpp>

namespace pitaya {

	namespace sc = boost::statechart;

	//! Grammar class.
	class Grammar {

		using PP = std::pair<ProductionID, ProductionID>;

	public:

		//! Constructor.
		/*!
			\param file Grammar file name.
		*/
		Grammar(const char* file);

		//! Number of symbols in this grammar.
		std::size_t symbol_count() const;

		//! Number of terminals in this grammar.
		std::size_t terminal_count() const;

		//! Number of productions in this grammar.
		std::size_t production_count() const;

		//! Get a symbol by id.
		Symbol& get_symbol(SymbolID);

		//! Get a production by id.
		Production& get_production(ProductionID);

		//! Get productions with same lhs.
		/*!
			\param id ID of the lhs.
			\return A pair indicating the start and the end production id.
		*/
		PP productions_by_lhs(SymbolID id);

		//! Get the end-mark symbol.
		Symbol& endmark();

		/// @cond
		void print_first_sets() const;
		/// @endcond

	private:

		std::vector<SharedSymbol> m_symbols;	//!< All symbols in this grammar.
		std::vector<Production> m_productions;	//!< All productions in this grammar.
		std::size_t m_terminal_count;			//!< Number of terminals in this grammar.

		//! Productions grouped by their lhs.
		std::unordered_map<SymbolID, PP> m_productions_by_lhs;

		//! Parse a grammar file.
		void read(const char* file);

		/// @cond
		void rearrange_symbols();
		void rearrange_productions();
		/// @endcond

		/// @cond boost
		// forward state declarations
		struct WaitForLHS;
		struct WaitForRHS;
		struct WaitForDecl;
		struct WaitForSyms;
		struct ReadComment;

		// event definitions
		struct EvGetSymbol : sc::event<EvGetSymbol> {};
		struct EvNextProduction : sc::event<EvNextProduction> {};
		struct EvStartDecl : sc::event<EvStartDecl> {};
		struct EvStartComm : sc::event<EvStartComm> {};

		// FSM definition
		struct ProductionReader : sc::state_machine<ProductionReader, WaitForLHS> {

			ProductionReader();

			ProductionID curr_pid;
			Associativity curr_assc;
			int curr_prec;
			bool token_decl;

			void read(std::ifstream&, std::vector<pitaya::Production>&);

		};

		// state definitions
		struct WaitForLHS : sc::simple_state<WaitForLHS, ProductionReader> {

			using reactions = boost::mpl::list<
				sc::custom_reaction<EvGetSymbol>,
				sc::custom_reaction<EvStartDecl>
			>;

			sc::result react(const EvGetSymbol&);
			sc::result react(const EvStartDecl&);

		};

		struct WaitForRHS : sc::simple_state<WaitForRHS, ProductionReader> {

			using reactions = boost::mpl::list<
				sc::custom_reaction<EvGetSymbol>,
				sc::transition<EvNextProduction, WaitForLHS>
			>;
			sc::result react(const EvGetSymbol&);

		};

		struct WaitForDecl : sc::simple_state<WaitForDecl, ProductionReader> {

			using reactions = boost::mpl::list<
				sc::custom_reaction<EvStartDecl>,
				sc::custom_reaction<EvStartComm>
			>;

			sc::result react(const EvStartDecl&);
			sc::result react(const EvStartComm&);

		};

		struct WaitForSyms : sc::simple_state<WaitForSyms, ProductionReader> {

			using reactions = boost::mpl::list<
				sc::custom_reaction<EvGetSymbol>,
				sc::transition<EvNextProduction, WaitForLHS>
			>;

			sc::result react(const EvGetSymbol&);

		};

		struct ReadComment : sc::simple_state<ReadComment, ProductionReader> {

			using reactions = boost::mpl::list<
				sc::transition<EvNextProduction, WaitForLHS>
			>;

		};
		/// @endcond

	};

}
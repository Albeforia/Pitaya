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

	/*!
		\ingroup General
		Grammar class.
	*/
	class Grammar {

		using PP = std::pair<ProductionID, ProductionID>;
		using SymbolIterator = std::vector<SharedSymbol>::iterator;

	public:

		//! Constructor.
		/*!
			\param file Grammar file name.
		*/
		Grammar(const std::string&);

		//! Number of symbols in this grammar.
		std::size_t symbol_count() const;

		//! Number of productions in this grammar.
		std::size_t production_count() const;

		//! Get a symbol by index.
		Symbol& get_symbol(std::size_t);

		//! Get a symbol by name.
		Symbol& get_symbol(std::string);

		//! Get a production by id.
		Production& get_production(ProductionID);

		//! Get productions with same lhs.
		/*!
			\return A pair indicating the start and the end production id.
		*/
		PP productions_by_lhs(const Symbol&);

		//! Get the end-mark symbol.
		Symbol& endmark();

		//! Get an iterator to the beginning of terminals.
		SymbolIterator terminal_begin();

		//! Get an iterator to the end of terminals.
		SymbolIterator terminal_end();

		//! Get an iterator to the beginning of nonterminals.
		SymbolIterator nonterminal_begin();

		//! Get an iterator to the end of nonterminals.
		SymbolIterator nonterminal_end();

		//! Get an iterator to the beginning of symbols.
		SymbolIterator symbol_begin();

		//! Get an iterator to the end of symbols.
		SymbolIterator symbol_end();

		/// @cond
		void print_first_sets() const;
		/// @endcond

	private:

		std::vector<SharedSymbol> m_symbols;	//!< All symbols in this grammar.
		std::vector<Production> m_productions;	//!< All productions in this grammar.

		SymbolIterator m_terminal_start;		//!< The beginning of terminals.
		SymbolIterator m_terminal_end;			//!< The end of terminals.
		SymbolIterator m_nonterminal_start;		//!< The beginning of nonterminals.
		SymbolIterator m_nonterminal_end;		//!< The end of nonterminals.

		//! Productions grouped by their lhs.
		std::unordered_map<Rank, PP> m_productions_by_lhs;

		//! Parse a grammar file.
		void read(const std::string&);

		/// @cond
		void rearrange_symbols();
		void rearrange_productions();
		void compute_lambdas();
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
			bool multi_decl;
			Symbol* curr_multi;

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
#pragma once

#include "Production.h"

#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/event.hpp>
#include <boost/statechart/custom_reaction.hpp>
#include <boost/mpl/list.hpp>

namespace pitaya {

	namespace sc = boost::statechart;

	//! Grammar class.
	class Grammar {

	public:

		//! Constructor.
		/*!
			\param file Grammar file name.
		*/
		Grammar(const char* file);

		//! Get a production by id.
		Production& get_production(std::size_t id);

		//! Number of productions in the grammar.
		std::size_t num_productions() const;

		//! Compute the first sets of every nonterminal.
		void compute_first_sets();

		/// @cond test

		void print_first_sets() const;

		/// @endcond

	private:

		std::vector<SharedSymbol> m_symbols;		//!< All symbols in this grammar.
		std::vector<Production> m_productions;		//!< All productions in this grammar.

		//! Parse a grammar file.
		void read(const char* file);

		/// @cond boost
		// forward state declarations
		struct WaitForLHS;
		struct WaitForRHS;

		// event definitions
		struct EvGetLHS : sc::event<EvGetLHS> {};
		struct EvGetRHS : sc::event<EvGetRHS> {};
		struct EvNextProduction : sc::event<EvNextProduction> {};

		// FSM definition
		struct ProductionReader : sc::state_machine<ProductionReader, WaitForLHS> {

			ProductionReader();

			std::size_t current;

			void read(std::ifstream&, std::vector<pitaya::Production>&);

		};

		// state definitions
		struct WaitForLHS : sc::simple_state<WaitForLHS, ProductionReader> {

			using reactions = boost::mpl::list<
				sc::custom_reaction<EvGetLHS>
			>;

			sc::result react(const EvGetLHS&);

		};

		struct WaitForRHS : sc::simple_state<WaitForRHS, ProductionReader> {

			using reactions = boost::mpl::list<
				sc::custom_reaction<EvNextProduction>,
				sc::custom_reaction<EvGetRHS>
			>;

			sc::result react(const EvNextProduction&);
			sc::result react(const EvGetRHS&);

		};
		/// @endcond

	};

}
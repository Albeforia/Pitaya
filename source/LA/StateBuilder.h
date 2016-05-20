#pragma once

#include "Grammar.h"
#include "State.h"

namespace pitaya {

	//! StateBuilder class.
	class StateBuilder {

	public:

		//! Constructor.
		StateBuilder(Grammar&);

		//! Build all states.
		void build();

		//! Get a state by id.
		const State& get_state(State::ID) const;

		/// @cond test
		void print_all() const;
		/// @endcond

	private:

		Grammar& m_grammar;		//!< The grammar this builder works on.

		std::unordered_set<State, boost::hash<State>> m_states;	//!< All states.
		std::vector<const State*> m_sorted;		//!< Sorted states for quick access.
		State m_curr_state;		//!< The State being built currently.

		//! Build a state according to m_curr_state.
		const State& build_state();

		//! Build all successors of an state.
		void build_successors(const State&);

		//! Decide token type for all final states.
		void decide_token_type();

	};

}
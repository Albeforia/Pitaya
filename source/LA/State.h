#pragma once

#include "BasicItem.h"

#include <vector>
#include <unordered_set>
#include <unordered_map>

#include <boost\functional\hash\hash.hpp>

namespace pitaya {

	//! State class.
	class State {

	public:

		friend class StateBuilder;

		using ID = std::size_t;
		//! <symbol_index, token_index>
		using Item = std::pair<std::size_t, std::size_t>;

		//! Constructor.
		State();

		//! Move constructor.
		State(State&&) noexcept;

		//! ID of the state.
		ID id() const;

		//! Add an item to basis.
		void add_base(const Symbol&, std::size_t token_index);

		//! Compute the closure of basis.
		void compute_closure(Grammar&);

		//! Whether this is a final state.
		bool& is_final() const;

		//! Token index of the state.
		std::size_t& token_index() const;

		//! Add a transition.
		void add_transition(const Symbol& symbol, const State&) const;

		//! Query a transition.
		/*!
			\param symbol Input symbol.
			\param to [out] Destination state.
			\return Whether the transition exists.
		*/
		bool transit(const Symbol& symbol, ID& to) const;

		//! Query a transition.
		/*!
			\param symbol Input symbol.
			\return Whether the transition exists.
		*/
		bool transit(const Symbol&) const;

		//! Sort the basis.
		void sort();

		//! Make the state empty.
		void reset();

		//! Two State are equal iff they have the same basis.
		friend bool operator==(const State&, const State&);

		//! Hash.
		friend std::size_t hash_value(const State&);

	private:

		ID m_id;								//!< ID of the state.
		std::vector<Item> m_basis;				//!< Basis of the state.

		//! Closure of the state.
		std::unordered_set<Item, boost::hash<Item>> m_closure;

		mutable bool m_is_final;				//!< Whether this is a final state.
		mutable std::size_t m_token_index;		//!< Token index of the state.
		mutable std::size_t m_final_index;		//!< Used in token type decision.

		//! State transitions.
		mutable std::unordered_map<Rank, ID> m_transitions;

		/// @cond
		//! Unique ID marking the first appearance of a state.
		static std::size_t order() {
			static std::size_t interned_ = 0;
			return interned_++;
		}
		/// @endcond

	};

}
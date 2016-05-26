#pragma once

#include <iostream>

namespace pitaya {

	/*!
		\ingroup SA
		Action type.
	*/
	enum class ActionType {

		SHIFT,
		REDUCE,
		GOTO,
		ACCEPT,
		ERROR,
		SSCONFLICT,		//!< A shift-shift conflict
		SRCONFLICT,		//!< A shift-reduce conflict
		RRCONFLICT,		//!< A reduce-reduce conflict

	};

	/*!
		\ingroup SA
		Action struct.
	*/
	struct Action {

		ActionType type;

		std::size_t value;	//!< StateID when type is SHIFT, ProductionID when type is REDUCE

		Action(ActionType, std::size_t value);

	};

	std::ostream& operator<<(std::ostream&, ActionType);

}
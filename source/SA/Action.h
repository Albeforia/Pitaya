#pragma once

#include "Symbol.h"
#include "Production.h"
#include "ItemSet.h"

namespace pitaya {

	enum class ActionType {

		SHIFT,
		REDUCE,
		ACCEPT,
		ERROR,
		SSCONFLICT,		//! A shift-shift conflict
		SRCONFLICT,		//! A shift-reduce conflict
		RRCONFLICT,		//! A reduce-reduce conflict

	};

	struct Action {

		ActionType type;
		//! StateID when type is SHIFT, ProductionID when type is REDUCE
		std::size_t value;

		Action(ActionType t, std::size_t v)
			: type {t}, value {v} {}

	};

}
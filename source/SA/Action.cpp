#include "Action.h"

#include <map>
#include <string>

namespace pitaya {

	Action::Action(ActionType type, std::size_t value)
		: type {type}, value {value} {}

	std::ostream& operator<<(std::ostream& os, ActionType type) {
		static std::map<ActionType, std::string> strings;
		if (strings.size() == 0) {
#define INSERT_ELEMENT(p) strings[p] = std::string(#p).substr(12)
			INSERT_ELEMENT(ActionType::SHIFT);
			INSERT_ELEMENT(ActionType::REDUCE);
			INSERT_ELEMENT(ActionType::GOTO);
			INSERT_ELEMENT(ActionType::ACCEPT);
			INSERT_ELEMENT(ActionType::ERROR);
			INSERT_ELEMENT(ActionType::SSCONFLICT);
			INSERT_ELEMENT(ActionType::SRCONFLICT);
			INSERT_ELEMENT(ActionType::RRCONFLICT);
#undef INSERT_ELEMENT
		}
		return os << strings[type];
	}

}
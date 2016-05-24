#include "Parser.h"

#include <cassert>
#include <fstream>

namespace pitaya {

	Parser::Parser(Grammar& grammar, ItemSetBuilder& builder)
		: m_grammar {grammar}, m_builder {builder} {}

	bool Parser::parse(Tokenizer& tokenizer) {
		std::ofstream file;
		file.open("report\\parse", std::ios::trunc);

		std::stack<StateID> state_stack {};
		state_stack.push(1);
		auto state = &m_builder.get_state(state_stack.top());
		while (tokenizer.has_next()) {
			auto& token = tokenizer.peek();
			auto symbol = &m_grammar.get_symbol(token.type);
			assert(*symbol != m_grammar.endmark());
			auto action = state->evaluate(*symbol);
			if (action.type == ActionType::ERROR) {
				// fallback
				if (symbol->type() == SymbolType::MULTITERMINAL) {
					symbol = &symbol->shared_terminal();
					action = state->evaluate(*symbol);
				}
			}
			bool stop = evaluate(action, state_stack, tokenizer);
			if (stop) {
				return action.type != ActionType::ERROR;
			}
			if (file.is_open()) {
				if (action.type == ActionType::SHIFT) {
					file << "SHIFT\t" << token.value << '(' << action.value << ")\n";
				}
				else if (action.type == ActionType::REDUCE) {
					file << "REDUCE\t" << m_grammar.get_production(action.value)
						<< '(' << state_stack.top() << ")\n";
				}
			}
			state = &m_builder.get_state(state_stack.top());
		}
		if (!tokenizer.has_next()) {
			auto action = state->evaluate(m_grammar.endmark());
			while (action.type != ActionType::ACCEPT) {
				if (action.type == ActionType::ERROR) {
					return false;
				}
				assert(action.type == ActionType::REDUCE);
				if (file.is_open()) {
					file << "REDUCE\t" << m_grammar.get_production(action.value)
						<< '(' << state_stack.top() << ")\n";
				}
				auto& p = m_grammar.get_production(action.value);
				for (size_t i = 0; i < p.rhs_count(); i++) {
					state_stack.pop();
				}
				auto& top = m_builder.get_state(state_stack.top());
				auto act = top.evaluate(p[0]);
				if (act.type == ActionType::ERROR) {
					return false;
				}
				assert(act.type == ActionType::GOTO);
				state_stack.push(act.value);
				state = &m_builder.get_state(state_stack.top());
				action = state->evaluate(m_grammar.endmark());
			}
		}

		file.close();
		return true;
	}

	bool Parser::evaluate(Action& action, std::stack<StateID>& stack, Tokenizer& tokenizer) {
		switch (action.type) {
			case ActionType::SHIFT:
				stack.push(action.value);
				tokenizer.next();
				break;
			case ActionType::REDUCE:
			{
				auto& p = m_grammar.get_production(action.value);
				for (size_t i = 0; i < p.rhs_count(); i++) {
					stack.pop();
				}
				auto& top = m_builder.get_state(stack.top());
				auto act = top.evaluate(p[0]);
				assert(act.type == ActionType::GOTO);
				stack.push(act.value);
			}
			break;
			case ActionType::ACCEPT:
			case ActionType::ERROR:
				return true;
			default:
				break;
		}
		return false;
	}

}
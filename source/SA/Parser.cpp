#include "Parser.h"

#include <cassert>

#define REPORT

#ifdef REPORT
#include <iostream>
#endif

namespace pitaya {

	Parser::Parser(Grammar& grammar, ItemSetBuilder& builder)
		: m_grammar {grammar}, m_builder {builder} {}

	bool Parser::parse(Tokenizer& tokenizer) {
		std::stack<StateID> state_stack {};
		state_stack.push(1);
		auto state = &m_builder.get_state(state_stack.top());
		while (tokenizer.has_next()) {
			auto& token = tokenizer.peek();
			auto symbol = &m_grammar.get_symbol(token.type);
			assert(*symbol != m_grammar.endmark());
			if (symbol->type() == SymbolType::MULTITERMINAL) {
				symbol = &symbol->shared_terminal();
			}
			auto action = state->evaluate(*symbol);
			bool stop = evaluate(action, state_stack, tokenizer);
			if (stop) {
				return action.type != ActionType::ERROR;
			}
#ifdef REPORT
			if (action.type == ActionType::SHIFT) {
				std::cout << "SHIFT\t" << token.value << '(' << action.value << ')' << std::endl;
			}
			else if (action.type == ActionType::REDUCE) {
				std::cout << "REDUCE\t" << m_grammar.get_production(action.value) << std::endl;
			}
#endif
			state = &m_builder.get_state(state_stack.top());
		}
		if (!tokenizer.has_next()) {
			auto action = state->evaluate(m_grammar.endmark());
			while (action.type != ActionType::ACCEPT) {
				if (action.type == ActionType::ERROR) {
					return false;
				}
				assert(action.type == ActionType::REDUCE);
#ifdef REPORT
				std::cout << "REDUCE\t" << m_grammar.get_production(action.value) << std::endl;
#endif
				auto& p = m_grammar.get_production(action.value);
				for (size_t i = 0; i < p.rhs_count(); i++) {
					state_stack.pop();
				}
				auto& top = m_builder.get_state(state_stack.top());
				auto act = top.evaluate(p[0]);
				if (act.type == ActionType::ERROR) {
					return false;
				}
				assert(act.type == ActionType::SHIFT);
				state_stack.push(act.value);
				state = &m_builder.get_state(state_stack.top());
				action = state->evaluate(m_grammar.endmark());
			}
		}
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
				assert(act.type == ActionType::SHIFT);
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
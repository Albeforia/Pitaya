#include "Grammar.h"
#include "StateBuilder.h"
#include "Tokenizer.h"
#include "ItemSetBuilder.h"
#include "Parser.h"

#include <memory>
#include <fstream>
#include <iostream>

using namespace pitaya;

int main() {

	auto la {std::make_unique<Grammar>("la.gram")};
	auto builder1 {std::make_unique<StateBuilder>(*la)};
	builder1->build();
	builder1->print_all();

	auto tokenizer {std::make_unique<Tokenizer>(*la, *builder1)};
	std::ifstream f;
	f.open("source.any");
	if (f.is_open()) {
		auto res = tokenizer->parse(f);
		if (!res.success) {
			std::cout << "[error] line " << res.err_line << ": " << res.err_input << std::endl;
		}
	}
	f.close();
	tokenizer->print_all();

	Symbol::clear();

	auto sa {std::make_unique<Grammar>("sa.gram")};
	auto builder2 {std::make_unique<ItemSetBuilder>(*sa)};
	builder2->build();
	builder2->print_all();

	auto parser {std::make_unique<Parser>(*sa,*builder2)};
	auto acc = parser->parse(*tokenizer);
	if (!acc) {
		std::cout << "syntax error!";
	}
	else {
		std::cout << "accept!";
	}

	return 0;

}
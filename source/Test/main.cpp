#include "Grammar.h"
#include "StateBuilder.h"
#include "Tokenizer.h"

#include <memory>
#include <fstream>

using namespace pitaya;

int main() {

	auto grammar {std::make_unique<Grammar>("la.gram")};
	auto builder {std::make_unique<StateBuilder>(*grammar)};
	builder->build();
	builder->print_all();
	auto tokenizer {std::make_unique<Tokenizer>(*grammar, *builder)};
	std::ifstream f;
	f.open("source.any");
	if (f.is_open()) {
		tokenizer->parse(f);
	}
	f.close();
	tokenizer->print_all();

	return 0;

}
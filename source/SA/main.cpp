#include "Grammar.h"
#include "ItemSetBuilder.h"

#include <memory>

using namespace pitaya;

int main() {

	auto pg {std::make_unique<Grammar>("sa.gram")};
	auto builder {std::make_unique<ItemSetBuilder>(*pg)};
	builder->build();
	builder->print_all();

	return 0;

}
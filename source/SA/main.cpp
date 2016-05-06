#include "Grammar.h"
#include "ItemSetBuilder.h"

#include <memory>

using namespace pitaya;

int main() {

	auto pg {std::make_shared<Grammar>("test.gram")};
	pg->compute_first_sets();
	pg->print_first_sets();

	auto builder {std::make_unique<ItemSetBuilder>(pg)};
	builder->build();
	builder->print_all();

	return 0;

}
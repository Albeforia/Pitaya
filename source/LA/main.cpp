#include "Grammar.h"
#include "StateBuilder.h"

#include <memory>

using namespace pitaya;

int main() {

	auto pg {std::make_unique<Grammar>("la.gram")};
	auto builder {std::make_unique<StateBuilder>(*pg)};
	builder->build();
	builder->print_all();

	return 0;

}
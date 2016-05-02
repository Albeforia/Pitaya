#include "Grammar.h"

#include <memory>

using namespace pitaya;

int main() {

	auto pg {std::make_unique<Grammar>("test.gram")};
	pg->compute_first_sets();

	return 0;

}
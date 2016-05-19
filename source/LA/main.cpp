#include "Grammar.h"

#include <memory>

using namespace pitaya;

int main() {

	auto pg {std::make_shared<Grammar>("la.gram")};

	return 0;

}
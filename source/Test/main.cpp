#include "Grammar.h"
#include "StateBuilder.h"
#include "Tokenizer.h"
#include "ItemSetBuilder.h"
#include "Parser.h"

#include <string>
#include <memory>
#include <fstream>
#include <iostream>

#include <boost\program_options.hpp>
namespace po = boost::program_options;

using namespace pitaya;

int main(int argc, char* argv[]) {

	po::options_description opt("Options");
	opt.add_options()
		("help,h", "show help message")
		("lexical", po::value<std::string>(), "lexical spec file")
		("syntax", po::value<std::string>(), "syntax spec file")
		("source,s", po::value<std::string>(), "source file")
		("silence", "do not report")
		("graph", "generate dot graph");

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, opt), vm);
	po::notify(vm);

	if (vm.count("help")) {
		std::cout << opt << std::endl;
		return 0;
	}

	if (vm.count("lexical") && vm.count("syntax") && vm.count("source")) {
		auto lexical {std::make_unique<Grammar>(vm["lexical"].as<std::string>())};
		auto builder1 {std::make_unique<StateBuilder>(*lexical)};
		builder1->build();
		if (!vm.count("silence")) {
			bool graph = vm.count("graph") != 0;
			builder1->report(graph);
		}

		auto tokenizer {std::make_unique<Tokenizer>(*lexical, *builder1)};
		std::ifstream f;
		f.open(vm["source"].as<std::string>());
		if (f.is_open()) {
			auto res = tokenizer->parse(f);
			if (!res.success) {
				std::cout << "[ERROR] line " << res.err_line << ": " << res.err_input << std::endl;
			}
		}
		f.close();
		if (!vm.count("silence")) {
			tokenizer->report();
		}

		Symbol::clear();

		auto syntax {std::make_unique<Grammar>(vm["syntax"].as<std::string>())};
		auto builder2 {std::make_unique<ItemSetBuilder>(*syntax)};
		builder2->build();
		if (!vm.count("silence")) {
			bool graph = vm.count("graph") != 0;
			builder2->report(graph);
		}

		auto parser {std::make_unique<Parser>(*syntax,*builder2)};
		auto acc = parser->parse(*tokenizer);
		if (!acc) {
			std::cout << "ERROR";
		}
		else {
			std::cout << "ACCEPT";
		}
	}
	else {
		std::cout << opt << std::endl;
	}

	return 0;

}
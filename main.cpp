#include <iostream>
#include "arg_parser.hpp"

using namespace std;

int main(int argc, char** argv)
{
	argparse::ArgumentParser parser("myapp");
	parser.add_argument("count")
		.help("Number of times")
		.default_value(1)
		.add_alias("c");
	
	parser.add_argument("verbose")
		.add_alias("v")
		.add_alias("debug")
		.help("Enable verbose output")
		.flag();

	try {
		auto args = parser.parse_args(argc, argv);
		int count = args.get<int>("count");
		bool verbose = args.get<bool>("verbose");

		cout << "Count: " << count << endl;
		cout << "Verbose: " << boolalpha << verbose << endl;
	}
	catch (const argparse::ArgumentError& e) {
		cerr << "Error: " << e.what() << "\n";
		cerr << parser.help();
	}

	return 0;
}

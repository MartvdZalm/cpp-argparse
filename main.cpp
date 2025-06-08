#include <iostream>
#include "arg_parser.hpp"
#include <windows.h>

int main(int argc, char** argv)
{
	argparse::ArgumentParser parser("myapp");

	parser
		.add_argument("color")
		.type_string()
		.help("Choose a color")
		.choices({ "RED", "BLUE", "GREEN" })
		.default_value("RED")
		.custom_validation(
			[](const std::string& val) {
				return std::all_of(val.begin(), val.end(), ::isupper);
			},
			"Color should be in uppercase."
		)
		.add_alias("c");

	try {
		auto args = parser.parse_args(argc, argv);
		std::string color = args.get<std::string>("color");
		std::cout << "Color: " << color << std::endl;
	}
	catch (const argparse::ArgumentError& e) {
		std::cerr << "Error: " << e.what() << "\n";
		return 1;
	}
	return 0;
}

#include <iostream>
#include "arg_parser.hpp"

int main(int argc, char** argv)
{
	argparse::ArgumentParser parser("myapp");
	parser.add_argument("port")
		.type_int()
		.min_value(1024)
		.max_value(49151)
		.default_value(8080)
		.help("Network port")
		.add_alias("p");

	parser.add_argument("color")
		.type_string()
		.choices({ "red", "blue", "green" })
		.help("Choose a color")
		.default_value("red");

	try {
		auto args = parser.parse_args(argc, argv);
		int port = args.get<int>("port");
		std::string color = args.get<std::string>("color");
		std::string apiKey = args.get<std::string>("api-key");

		std::cout << "Port: " << port << std::endl;
		std::cout << "Color: " << color << std::endl;
	}
	catch (const argparse::ArgumentError& e) {
		std::cerr << "Error: " << e.what() << "\n";
		return 1;
	}
	return 0;
}

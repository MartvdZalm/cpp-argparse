#include <iostream>
#include "arg_parser.hpp"

int main(int argc, char** argv) {
    argparse::ArgumentParser parser("basic_usage");

    parser.add_argument("color")
        .type_string()
        .help("Color to use")
        .choices({ "RED", "GREEN", "BLUE" })
        .default_value("RED")
        .add_alias("c");

    parser.add_argument("count")
        .type_int()
        .help("Number of times to repeat")
        .required()
        .min_value(1)
        .max_value(10);

    parser.add_argument("debug")
        .type_bool()
        .help("Enable debug mode")
        .flag()
        .add_alias("d");

    try {
        auto args = parser.parse_args(argc, argv);

        std::string color = args.get<std::string>("color");
        int count = args.get<int>("count");
        bool debug = args.get<bool>("debug");

        std::cout << "Color: " << color << "\n";
        std::cout << "Count: " << count << "\n";
        std::cout << "Debug: " << std::boolalpha << debug << "\n";

        for (int i = 0; i < count; ++i) {
            std::cout << "Hello in " << color << "!\n";
        }
    }
    catch (const argparse::ArgumentError& e) {
        std::cerr << "Argument error: " << e.what() << "\n";
        return 1;
    }
}

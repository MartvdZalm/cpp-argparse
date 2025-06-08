#pragma once

#include <string>
#include <vector>
#include <map>
#include <stdexcept>
#include <functional>
#include <variant>
#include <optional>
#include <algorithm>
#include <cstdlib>
#include <sstream>

namespace argparse
{
	class ArgumentError : public std::runtime_error
	{
	public:
		using std::runtime_error::runtime_error;
	};

	/**
	 * @brief Represents a command-line argument with metadata and validation.
	 *
	 * Supports various data types, default values,
	 * choices, flags, and custom validation logic.
	 */
	class Argument
	{
	public:
		/**
		 * @brief Enumerates supported argument types.
		 */
		enum class ArgType { INT, FLOAT, STRING, BOOL, AUTO };

	private:
		std::string name_;
		std::vector<std::string> aliases_;
		std::string help_;
		bool required_ = false;
		std::variant<int, float, std::string, bool> default_value_;
		bool is_flag_ = false;
		std::optional<int> min_value_;
		std::optional<int> max_value_;
		std::optional<std::string> env_var_;
		std::vector<std::string> choices_;
		ArgType type_ = ArgType::AUTO;
		std::function<bool(const std::string&)> custom_validator_;
		std::optional<std::string> custom_validator_error_;

		static std::string normalize_name(std::string name)
		{
			if (name.starts_with("--")) return name.substr(2);
			if (name.starts_with("-")) return name.substr(1);
			return name;
		}

	public:
		/**
		 * @brief Constructs an Argument with a given name.
		 * @param name The name of the argument (e.g., "input", "verbose").
		 */
		Argument(std::string name) : name_(std::move(name)) {}

		/**
		 * @brief Sets the help message shown in the generated help text.
		 * @param text Help description.
		 * @return Reference to the current Argument instance.
		 */
		Argument& help(std::string text) { help_ = std::move(text); return *this; }

		/**
		 * @brief Marks the argument as required.
		 * @param req Set to true if the argument is required (default: true).
		 * @return Reference to the current Argument instance.
		 */
		Argument& required(bool req = true) { required_ = req; return *this; }

		/**
		 * @brief Adds an alias (short name) for the argument.
		 * @param alias The alias (e.g., "v" for verbose).
		 * @return Reference to the current Argument instance.
		 * @throws ArgumentError if alias is empty or duplicates the name.
		 */
		Argument& add_alias(std::string alias)
		{
			alias = normalize_name(std::move(alias));
			if (alias.empty() || alias == name_) {
				throw ArgumentError("Invalid alias");
			}
			aliases_.push_back(std::move(alias));
			return *this;
		}


		bool is_required() const
		{
			return required_;
		}

		const std::string& name() const
		{
			return name_;
		}

		const std::vector<std::string>& aliases() const
		{
			return aliases_;
		}

		const std::string& help() const
		{
			return help_;
		}

		const ArgType type() const
		{
			return type_;
		}

		bool is_flag() const
		{
			return is_flag_;
		}

		const auto& default_value() const
		{
			return default_value_;
		}

		const std::vector<std::string>& choices() const
		{
			return choices_;
		}

		std::optional<std::string> get_env_value() const
		{
			if (!env_var_) return std::nullopt;
			const char* env_value = std::getenv(env_var_->c_str());
			if (env_value) {
				std::cout << "Found env value: " << env_value << std::endl;
				return std::string(env_value);
			}
			std::cout << "Environment variable " << *env_var_ << " not found!" << std::endl;
			return std::nullopt;
		}

		/**
		 * @brief Sets a default value for the argument.
		 * @tparam T Type of the value (int, float, string, bool).
		 * @param val The default value.
		 * @return Reference to the current Argument instance.
		 */
		template<typename T>
		Argument& default_value(T val)
		{
			default_value_ = val;
			return *this;
		}

		/**
		 * @brief Treats this argument as a flag (boolean switch).
		 * @param set Whether to enable flag behavior (default: true).
		 * @return Reference to the current Argument instance.
		 */
		Argument& flag(bool set = true)
		{
			is_flag_ = set;
			if (set) default_value_ = false;
			return *this;
		}

		/**
		 * @brief Sets the minimum value (only applies to int type).
		 * @param val The minimum allowed value.
		 * @return Reference to the current Argument instance.
		 */
		Argument& min_value(int val)
		{
			min_value_ = val;
			return *this;
		}

		/**
		 * @brief Sets the maximum value (only applies to int type).
		 * @param val The maximum allowed value.
		 * @return Reference to the current Argument instance.
		 */
		Argument& max_value(int val)
		{
			max_value_ = val;
			return *this;
		}

		/**
		 * @brief Sets an environment variable to use as a fallback value.
		 * @param var_name Name of the environment variable.
		 * @return Reference to the current Argument instance.
		 */
		Argument& env(const std::string& var_name)
		{
			env_var_ = var_name;
			return *this;
		}

		/**
		 * @brief Restricts acceptable values to a list of choices.
		 * @param options A list of valid string options.
		 * @return Reference to the current Argument instance.
		 */
		Argument& choices(std::vector<std::string> options)
		{
			choices_ = std::move(options);
			return *this;
		}

		/**
		 * @brief Specifies that the argument expects an integer value.
		 * @return Reference to the current Argument instance.
		 */
		Argument& type_int()
		{
			return set_type<int>(ArgType::INT, 0);
		}

		/**
		 * @brief Specifies that the argument expects a float value.
		 * @return Reference to the current Argument instance.
		 */
		Argument& type_float()
		{
			return set_type<float>(ArgType::FLOAT, 0.0f);
		}

		/**
		 * @brief Specifies that the argument expects a string value.
		 * @return Reference to the current Argument instance.
		 */
		Argument& type_string()
		{
			return set_type<std::string>(ArgType::STRING, std::string());
		}

		/**
		 * @brief Specifies that the argument expects a boolean value.
		 * @return Reference to the current Argument instance.
		 */
		Argument& type_bool()
		{
			return set_type<bool>(ArgType::BOOL, false);
		}

		template<typename T>
		Argument& set_type(ArgType arg_type, T default_val)
		{
			type_ = arg_type;
			if (std::holds_alternative<T>(default_value_)) return *this;
			default_value_ = default_val;
			return *this;
		}

		/**
		 * @brief Adds a custom validation function.
		 * @param validate_fn A function that returns true if valid.
		 * @param error_message Message to show if validation fails.
		 * @return Reference to the current Argument instance.
		 */
		Argument& custom_validation(std::function<bool(const std::string&)> validate_fn, const std::string& error_message)
		{
			custom_validator_ = std::move(validate_fn);
			custom_validator_error_ = error_message;
			return *this;
		}

		/**
		 * @brief Validates the provided string value.
		 * @param value_str The string to validate.
		 * @throws ArgumentError if validation fails.
		 */
		void validate(const std::string& value_str) const
		{
			if (type_ == ArgType::INT) {
				if (value_str.empty()) throw ArgumentError("Missing integer value");
				try {
					int value = std::stoi(value_str);
					if (min_value_ && value < *min_value_) {
						throw ArgumentError("Value must be > " + std::to_string(*min_value_));
					}
					if (max_value_ && value > *max_value_) {
						throw ArgumentError("Value must be < " + std::to_string(*max_value_));
					}
				}
				catch (...) {
					throw ArgumentError("Invalid integer value: " + value_str);
				}
			}
			else if (!choices_.empty()) {
				if (std::find(choices_.begin(), choices_.end(), value_str) == choices_.end()) {
					throw ArgumentError("Invalid choice. Options: " + join_strings(choices_, ", "));
				}
			}
				
			if (custom_validator_ && !custom_validator_(value_str)) {
				throw ArgumentError(custom_validator_error_.value_or("Validation failed."));
			}
		}

		// Utility function:
		/**
		 * @brief Joins a vector of strings with a delimiter.
		 * @param vec The vector of strings.
		 * @param delim The delimiter string.
		 * @return A concatenated string.
		 */
		static std::string join_strings(const std::vector<std::string>& vec, const std::string& delim)
		{
			std::string result;
			for (size_t i = 0; i < vec.size(); ++i) {
				if (i != 0) result += delim;
				result += vec[i];
			}
			return result;
		}
	};

	class ParsedArgs
	{
		std::map<std::string, std::variant<int, float, std::string, bool>> values_;
	public:
		template<typename T>
		T get(const std::string& name) const
		{
			std::string key = name.starts_with("--") ? name.substr(2) :
				name.starts_with("-") ? name.substr(1) : name;
			return std::get<T>(values_.at(key));
		}

		friend class ArgumentParser;
	};

	/**
	 * @brief Command-line argument parser.
	 *
	 * This class allows you to define and parse command-line arguments for your application.
	 *
	 * Example usage:
	 * @code
	 * argparse::ArgumentParser parser("myapp");
	 * parser.add_argument("mode")
	 *       .help("Mode of operation")
	 *       .choices({ "debug", "release" });
	 *
	 * auto args = parser.parse_args(argc, argv);
	 * std::string mode = args.get<std::string>("mode");
	 * @endcode
	 */
	class ArgumentParser
	{
		bool auto_help_ = true;
		std::string prog_name_;
		std::vector<Argument> args_;
		std::unordered_map<std::string, size_t> arg_index_;

		void build_lookup()
		{
			arg_index_.clear();
			for (size_t i = 0; i < args_.size(); ++i) {
				const auto& arg = args_[i];
				arg_index_[arg.name()] = i;
				for (const auto& alias : arg.aliases()) {
					if (arg_index_.count(alias)) {
						throw ArgumentError("Duplicate alias: -" + alias);
					}
					arg_index_[alias] = i;
				}
			}
		}

		std::string format_help_line(const Argument& arg) const
		{
			std::ostringstream oss;
			oss << "--" << arg.name();
			for (const auto& alias : arg.aliases()) {
				oss << ", -" << alias;
			}
			oss << "\t" << arg.help();

			if (std::holds_alternative<std::string>(arg.default_value())) {
				oss << " [default: " << std::get<std::string>(arg.default_value()) << "]";
			}
			else if (std::holds_alternative<int>(arg.default_value())) {
				oss << " [default: " << std::get<int>(arg.default_value()) << "]";
			}
			else if (std::holds_alternative<float>(arg.default_value())) {
				oss << " [default: " << std::get<float>(arg.default_value()) << "]";
			}
			else if (std::holds_alternative<bool>(arg.default_value())) {
				oss << " [default: " << std::get<bool>(arg.default_value()) << "]";
			}

			if (!arg.choices().empty()) {
				oss << " (choices: " << Argument::join_strings(arg.choices(), ", ") << ")";
			}

			if (arg.is_required()) {
				oss << " (required)";
			}

			return oss.str();
		}

	public:
		/**
		 * @brief Create a new ArgumentParser with the given program name.
		 * @param prog_name The name of the program, typically argv[0].
		 */
		explicit ArgumentParser(std::string prog_name) : prog_name_(std::move(prog_name)) {}

		/**
		 * @brief Enable or disable automatic help display.
		 * @param enable If true, displays help if no arguments or --help/-h is provided.
		 * @return Reference to this parser.
		 */
		ArgumentParser& auto_help(bool enable)
		{
			auto_help_ = enable;
			return *this;
		}

		/**
		 * @brief Add a new command-line argument.
		 *
		 * Creates a new `Argument` object and returns a reference to it so you can chain configuration calls.
		 *
		 * @param name The name of the argument (e.g., "input" or "--file").
		 * @return Reference to the newly created Argument object.
		 */
		Argument& add_argument(std::string name)
		{
			args_.emplace_back(std::move(name));
			return args_.back();
		}

		/**
		 * @brief Get a formatted help string showing all arguments and their descriptions.
		 * @return The help text as a string.
		 */
		std::string help() const
		{
			std::string help_text = "Usage: " + prog_name_ + " [OPTIONS]\n\nOptions:\n";
			for (const auto& arg : args_) {
				help_text += "  " + format_help_line(arg) + "\n";
			}
			return help_text;
		}

		/**
		 * @brief Parse the command-line arguments.
		 *
		 * Parses `argc` and `argv`, applying validations, default values, environment fallbacks, etc.
		 * If `--help` or no arguments are passed and `auto_help` is enabled, it prints help and exits.
		 *
		 * @param argc Argument count (from `main`).
		 * @param argv Argument values (from `main`).
		 * @return A ParsedArgs object to retrieve argument values by name.
		 * @throws ArgumentError If any validation fails or required argument is missing.
		 */
		ParsedArgs parse_args(int argc, char** argv)
		{
			build_lookup();
			std::unordered_map<std::string, std::string> provided_args;
			ParsedArgs result;

			if (auto_help_ && argc == 1) {
				std::cout << help();
				std::exit(0);
			}
			else {
				for (int i = 1; i < argc; ++i) {
					if (std::string(argv[i]) == "--help" || std::string(argv[i]) == "-h") {
						std::cout << help();
						std::exit(0);
					}
				}
			}
			
			for (int i = 1; i < argc; ++i) {
				std::string arg = argv[i];
				std::string name, value;

				if (arg.starts_with("--")) {
					name = arg.substr(2);
					if (!arg_index_.count(name)) {
						throw ArgumentError("Unrecognized argument: --" + name);
					}
					if (i + 1 < argc && !std::string(argv[i + 1]).starts_with("-")) {
						value = argv[++i];
					}
					else if (!arg_index_.count(name) || !args_[arg_index_.at(name)].is_flag()) {
						throw ArgumentError("Missing value for --" + name);
					}
				}
				else if (arg.starts_with("-")) {
					name = arg.substr(1);
					if (!arg_index_.count(name)) {
						throw ArgumentError("Unrecognized argument: -" + name);
					}
					if (i + 1 < argc && !std::string(argv[i + 1]).starts_with("-")) {
						value = argv[++i];
					}
					else if (!arg_index_.count(name) || !args_[arg_index_.at(name)].is_flag()) {
						throw ArgumentError("Missing value for -" + name);
					}
				}

				if (!name.empty() && arg_index_.count(name)) {
					provided_args[args_[arg_index_.at(name)].name()] = std::move(value);
				}
			}

			for (const auto& arg : args_) {
				const std::string& name = arg.name();
				result.values_[name] = arg.default_value();

				if (provided_args.count(name)) {
					const std::string& value_str = provided_args.at(name);
					arg.validate(value_str);
					result.values_[name] = convert_value(value_str, arg.type());
				}
				else if (auto env_val = arg.get_env_value()) {
					arg.validate(*env_val);
					result.values_[name] = convert_value(*env_val, arg.type());
				}
				else if (arg.is_required()) {
					throw ArgumentError("Missing required argument: --" + name);
				}
			}

			return result;
		}

	private:
		std::variant<int, float, std::string, bool> convert_value(const std::string& value_str, Argument::ArgType type)
		{
			try {
				switch (type)
				{
					case Argument::ArgType::INT: return std::stoi(value_str);
					case Argument::ArgType::FLOAT: return std::stof(value_str);
					case Argument::ArgType::BOOL: return (value_str == "true" || value_str == "1");
					case Argument::ArgType::STRING: return value_str;
					case Argument::ArgType::AUTO: {
						if (value_str == "true" || value_str == "false" ||
							value_str == "1" || value_str == "0") {
							return value_str == "true" || value_str == "1";
						}

						try {
							size_t pos;
							int i = std::stoi(value_str, &pos);
							if (pos == value_str.size()) return i;
						}
						catch (...) {}

						try {
							size_t pos;
							float f = std::stof(value_str, &pos);
							if (pos == value_str.size()) return f;
						}
						catch (...) {}

						return value_str;
					}
					default: return value_str;
				}
			}
			catch (...) {
				throw ArgumentError("Invalid value format");
			}
		}
	};
}
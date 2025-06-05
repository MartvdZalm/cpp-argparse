#pragma once

#include <string>
#include <vector>
#include <map>
#include <stdexcept>
#include <functional>
#include <variant>

namespace argparse
{
	class ArgumentError : public std::runtime_error
	{
	public:
		using std::runtime_error::runtime_error;
	};

	class Argument
	{
		std::string name_;
		std::vector<std::string> aliases_;
		std::string help_;
		bool required_ = false;
		std::variant<int, float, std::string, bool> default_;
		bool is_flag_ = false;

		static std::string normalize_name(std::string name)
		{
			if (name.starts_with("--")) {
				return name.substr(2);
			}
			if (name.starts_with("-")) {
				return name.substr(1);
			}
			return name;
		}

	public:
		Argument(std::string name) : name_(std::move(name)) {}

		Argument& help(std::string text) { help_ = std::move(text); return *this; }
		Argument& required(bool req = true) { required_ = req; return *this; }

		Argument& add_alias(std::string alias)
		{
			alias = normalize_name(std::move(alias));
			if (alias.empty() || alias == name_) {
				throw ArgumentError("Invalid alias");
			}
			aliases_.push_back(std::move(alias));
			return *this;
		}


		bool is_required() const { return required_; }
		const std::string& name() const { return name_; }
		const std::vector<std::string>& aliases() const { return aliases_; }
		const std::string& help() const { return help_; }
		bool is_flag() const { return is_flag_; }
		const auto& default_value() const { return default_; }

		template<typename T>
		Argument& default_value(T val)
		{
			default_ = val;
			return *this;
		}

		Argument& flag(bool set = true)
		{
			is_flag_ = set;
			if (set) default_ = false;
			return *this;
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

	class ArgumentParser
	{
		std::string prog_name_;
		std::vector<Argument> args_;
		std::unordered_map<std::string, std::string> arg_lookup_;

		void build_lookup()
		{
			arg_lookup_.clear();
			for (const auto& arg : args_) {
				arg_lookup_[arg.name()] = arg.name();
				for (const auto& alias : arg.aliases()) {
					if (arg_lookup_.count(alias)) {
						throw ArgumentError("Duplicate alias: -" + alias);
					}
					arg_lookup_[alias] = arg.name();
				}
			}
		}

		std::string format_help_line(const Argument& arg) const
		{
			std::string line;

			if (!arg.aliases().empty()) {
				for (const auto& alias : arg.aliases()) {
					line += "-" + alias + ", ";
				}
			}

			line += "--" + arg.name();

			if (!arg.help().empty()) {
				line += "\t" + arg.help();
			}

			return line;
		}

	public:
		explicit ArgumentParser(std::string prog_name) : prog_name_(std::move(prog_name)) {}

		Argument& add_argument(std::string name)
		{
			args_.emplace_back(std::move(name));
			return args_.back();
		}

		std::string help() const
		{
			std::string help_text = "Usage: " + prog_name_ + " [OPTIONS]\n\nOptions:\n";
			for (const auto& arg : args_) {
				help_text += "  " + format_help_line(arg) + "\n";
			}
			return help_text;
		}

		ParsedArgs parse_args(int argc, char** argv)
		{
			build_lookup();

			std::unordered_map<std::string, std::string> provided_args;
			ParsedArgs result;

			for (int i = 1; i < argc; ++i) {
				std::string arg = argv[i];
				std::string name;
				std::string value;

				if (arg.starts_with("--")) {
					name = arg.substr(2);
					if (i + 1 < argc && !std::string(argv[i + 1]).starts_with("-")) {
						value = argv[++i];
					}
				}
				else if (arg.starts_with("-")) {
					name = arg.substr(1);
					if (i + 1 < argc && !std::string(argv[i + 1]).starts_with("-")) {
						value = argv[++i];
					}
				}

				if (!name.empty()) {
					if (!arg_lookup_.count(name)) {
						throw ArgumentError("Unknown argument: " + arg);
					}
					provided_args[arg_lookup_.at(name)] = std::move(value);
				}
			}

			for (const auto& arg : args_) {
				const std::string& name = arg.name();

				if (arg.is_required() && !provided_args.count(name)) {
					throw ArgumentError("Missing required argument: --" + name);
				}

				if (provided_args.count(name)) {
					const std::string& value_str = provided_args.at(name);

					if (arg.is_flag()) {
						result.values_[name] = true;
					}
					else {
						try {
							if (std::holds_alternative<int>(arg.default_value())) {
								result.values_[name] = std::stoi(value_str);
							}
							else if (std::holds_alternative<float>(arg.default_value())) {
								result.values_[name] = std::stof(value_str);
							}
							else if (std::holds_alternative<std::string>(arg.default_value())) {
								result.values_[name] = value_str;
							}
							else if (std::holds_alternative<bool>(arg.default_value())) {
								result.values_[name] = (value_str == "true" || value_str == "1");
							}
						}
						catch (...) {
							throw ArgumentError("Invalid value for argument: --" + name);
						}
					}
				}
				else {
					result.values_[name] = arg.default_value();
				}
			}

			return result;
		}
	};
}

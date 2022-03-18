#include <iostream>
#include <fstream>

#include "utils/logger.h"
#include <nlohmann/json.hpp>
#include "ConcreateMediator.h"
#include <bits/stdc++.h>

#include <cxxopts.hpp>
#include <cmath>

cxxopts::ParseResult ParseArgs(int argc, char **argv) {
	try {
		cxxopts::Options options(PROJECT_NAME, "Software version of PTS2");
		options.positional_help("[optional args]").show_positional_help();
		auto log_path = fmt::format("/var/log/{}/core.log", PROJECT_NAME);
		options.allow_unrecognised_options()
			.add_options()
				("v,version", "Print version")
				("d,debug", "Force debug trace")
				("syslog", "Use syslog instead of file+stdout logging")
#ifdef PROJECT_NAME
				("log-path", "Path to log file", cxxopts::value<std::string>()->default_value(log_path))
#else
    #error No project name in definitions
#endif
				("cfg-path", "Path to config file", cxxopts::value<std::string>()->default_value("./config.json"))
				("db-path", "Path to DB file", cxxopts::value<std::string>()->default_value("./DB/data.sqlite"))
				("h,help", "Print help");

		auto result = options.parse(argc, argv);

		if (result.count("help")) {
			std::cout << options.help({"", "Group"}) << std::endl;
			exit(0);
		}

		return result;
	} catch (const cxxopts::OptionException &e) {
		std::cout << "error parsing options: " << e.what() << std::endl;
		exit(1);
	}
}

auto getConfig(const std::string& path) -> std::string {
    std::ifstream t(path, std::ios_base::in);

	if (t.is_open()) {
		return {(std::istreambuf_iterator<char>(t)),
                std::istreambuf_iterator<char>()};
	} else {
        LogPrintf(spdlog::level::err, "No config file found");
    }
	return {};
}

float roundoff1(float value, unsigned char prec) {
    float pow_10 = pow(10.0f, (float)prec);
    return round(value * pow_10) / pow_10;
}

float roundPrecission(float var) {
    // we use array of chars to store number
    // as a string.
    char str[40];

    // Print in string the value of var
    // with two decimal point
    sprintf(str, "%.2f", var);

    // scan string value in var
    sscanf(str, "%f", &var);

    return var;
}

int main(int argc, char **argv) {
    auto args = ParseArgs(argc, argv);
    auto res = LogOpen(args["log-path"].as<std::string>(), args.count("debug"), args.count("syslog"));
    if(res == -1) {
        return -1;
    }
    const auto cfgStr = getConfig(args["cfg-path"].as<std::string>());
    auto cfgObj = nlohmann::json::parse(cfgStr);

    LogPrintf(spdlog::level::info, "|VERSION|: 2022022303");
    auto mediator = ConcreteMediator::create(cfgObj);
    mediator->setAllMediator();
    mediator->start();
    mediator->end();

    return 0;
}

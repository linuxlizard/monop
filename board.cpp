//
// Created by dpoole on 6/24/24.
//
#include <string>
#include <vector>
#include <fmt/format.h>
#include <ranges>
#include <fstream>

#include "board.h"

Property::Property(std::string name, std::vector<uint> values)
		: name(std::move(name))
{
	if (values.size() != 9) {
		std::string msg = fmt::format("expected 9 values but found {}", values.size());
		throw std::runtime_error(msg);
	}

	this->purchase_price = values[0];
	this->house_cost = values[1];
	std::copy(values.begin() + 2, values.end(), this->rents.begin());
}

Space parse_board_line(const std::string &line) {
	// https://www.youtube.com/watch?v=V14xGZAyVKI
	auto split_strings = std::ranges::views::split(line, ',');

	std::vector<std::string> tokens;

	for (const auto &rng: split_strings) {
		tokens.emplace_back(rng.begin(), rng.end());
	}

//	fmt::print("size={}\n", tokens.size());

	tokens.clear();
	std::ranges::copy(split_strings | std::ranges::views::transform([](auto &&rng) {
		std::string s(rng.begin(), rng.end());
		return s;
	}), std::back_inserter(tokens));
//	fmt::print("found len={} tokens. First={}\n", tokens.size(), tokens[0]);

	if (tokens.size() == 10) {
		// we found a property; convert rest of tokens into integers
		std::vector<uint> values;
		try {
			std::ranges::copy(tokens | std::views::drop(1) | std::views::transform([](auto &&s) -> int {
//				fmt::print("stoi({})\n", s);
				return std::stoi(s);
			}), std::back_inserter(values));
		}
		catch (std::invalid_argument const &err) {
			std::string msg = fmt::format("property {} has invalid argument {}\n", tokens[0], err.what());
			throw std::runtime_error(msg);
		}

//		Space space { std::in_place_type<Property>, tokens[0], values };

		//fmt::print("{}\n", std::get<Property>(board.back()).name);
//		auto p = std::get<Property>(space);
//		fmt::print("{} price={} hotel rent={}\n", p.name, p.get_purchase_price(), p.get_rent(HOTEL));

		return Space { std::in_place_type<Property>, tokens[0], values };
	}

	if (tokens[0] == "Community Chest") {
		return CommunityChest();
	}
	if (tokens[0] == "Chance") {
		return Chance();
	}
	if (tokens[0] == "Income Tax") {
		return Penalty(tokens[0], PENALTY_INCOME_TAX);
	}

	if (tokens[0] == "Luxury Tax") {
		return Penalty(tokens[0], PENALTY_LUXURY_TAX);
	}

	fmt::print("railroad? {} {}\n", tokens[0].at(tokens[0].size() - 1), tokens[0].at(tokens[0].size() - 2));

	if (tokens[0].at(tokens[0].size() - 1) == 'R' && tokens[0].at(tokens[0].size() - 2) == 'R') {
		return Railroad(tokens[0]);
	}

	if (tokens[0] == "Jail") {
		return Utility(tokens[0]);
	}

	if (tokens[0] == "Go To Jail") {
		return Penalty(tokens[0], PENALTY_GO_TO_JAIL);
	}

	if (tokens[0] == "Electric Company" || tokens[0] == "Water Works") {
		return Utility(tokens[0]);
	}

	if (tokens[0] == "Free Parking") {
		return Utility(tokens[0]);
	}

	if (tokens[0] == "Go") {
		return Space { std::in_place_type<GoCard>, tokens[0] };
	}

	auto msg = fmt::format("unidentified card \"{}\"", tokens[0]);
	throw std::runtime_error(msg);
}

std::vector<Space> load_board(const std::string &file_name)
{
	std::ifstream input_file(file_name);

	if (input_file.fail()) {
		auto msg = fmt::format("failed to open {} {}", file_name, std::strerror(errno));
		throw std::runtime_error(msg);
	}

	std::vector<Space> board;

	std::string line;
	while (std::getline(input_file, line)) {
		Space s = parse_board_line(line);
		board.push_back(std::move(s));
		Space& last = board.back();
		if (const Property *p = std::get_if<Property>(&last) ) {
			fmt::print("add property {} to the board\n", p->name);
		}

		// https://en.cppreference.com/w/cpp/utility/variant/visit
//		var_t w = std::visit([](auto&& arg) -> var_t { return arg + arg; }, v);
		std::string name = std::visit([](auto&& arg) -> std::string { return arg.name; }, s);
		fmt::print("add {} to the board\n", name);
	}

	return board;
}

std::string get_name(const Space& space) {
	return std::visit([](auto &&arg) -> std::string { return arg.name; }, space);
}
// davep 20240615 I'm bored. Let's play with C++ and write a monopoly simulator.

// After having played with Rust for a while, can I do C++ without any dynamic memory?


#include <iostream>
#include <fstream>
#include <random>
#include <cstring>
#include <cerrno>
#include <ranges>
#include <vector>
#include <functional>
#include <string_view>
#include <utility>
#include <fmt/format.h>
#include <cassert>

#include "board.h"
#include "player.h"


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
			fmt::print(stderr, "property {} has invalid argument {}\n", tokens[0], err.what());
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
			fmt::print("add {} to the board\n", p->name);
		}
	}

	return board;
}

void calculate_penalty(Penalty& penalty, Player& player)
{
	switch (penalty.penalty_type) {
		case PENALTY_INCOME_TAX:
			player.pay_tax(200);
			break;

		case PENALTY_LUXURY_TAX:
			player.pay_tax(100);
			break;

		case PENALTY_GO_TO_JAIL:
			player.go_to_jail();
			break;

		default:
			std::string msg = fmt::format("{} unhandled PenaltyType {}",
										  __func__, static_cast<int>(penalty.penalty_type));
			throw std::runtime_error(msg);
	}
}

int main()
{
	std::cout << "Hello, World!" << std::endl;

	const std::string board_filename { "/home/dpoole/src/monop/board.txt" };
	std::vector<Space> board = load_board(board_filename);

	assert(board.size() == BOARD_SIZE && "The board must contain 40 elements.");

	std::vector<Player> player_list;

	player_list.emplace_back( "Dave", 1500 );
	player_list.emplace_back( "Sarah", 1500 );
	player_list.emplace_back( "Matthew", 1500 );
	player_list.emplace_back( "Brendan", 1500 );

	for ( uint i=0 ; i<100 ; i++ ) {
		for ( uint player_idx=0 ; player_idx<player_list.size() ; player_idx++ ) {

			while (true) {
				Player &player = player_list.at(player_idx);
				auto roll = player.roll();
				fmt::print("Player {} rolled {} {}\n", player.name, roll.first, roll.second);

				bool rolled_doubles = roll.first == roll.second;
				if (rolled_doubles) {
					// TODO goto jail on rolling three doubles
					fmt::print("{} rolled doubles!\n", player.name);
				}

				uint pos = player.move();

				Space& land = board.at(pos);
				std::visit([&player](auto &&arg) { fmt::print("player {} landed on {}\n", player.name, arg.name); }, land);

				if (player.passed_go()) {
					fmt::print("player {} has passed Go and earns 200!\n", player.name);
					player.add_money(200);
				}

				if (std::holds_alternative<Property>(land)) {
					auto& property = std::get<Property>(land);

					std::optional<uint> owner = get_owner<Property>(property);

					if (owner) {
						auto& owner_player = player_list.at(owner.value());
						fmt::print("{} is owned by {}\n", property.name, owner_player.name);
						if ( owner != player_idx ) {
							// owner is not self so rent must be paid
							uint rent = property.get_rent(NO_HOUSES);
							fmt::print("{} must pay {} {} in rent\n", player.name, owner_player.name, rent);
							owner_player.earn_rent( player.pay_rent(rent));
						}
					}
					else {
						fmt::print("{} is unowned\n", property.name);

						if (player.buys(property)) {
							property.set_owner(player_idx);
						}
					}
				}
				else if (std::holds_alternative<Penalty>(land)) {
					auto& penalty = std::get<Penalty>(land);

					fmt::print("{} lands on a penalty {}\n", player.name, penalty.name);

					calculate_penalty(penalty,player);
				}

				if (rolled_doubles) {
					continue;
				}

				fmt::print("{} has completed their turn\n", player.name);
				break;
			}
		}
	}

	fmt::print("game is over!\n");

	for (Player& player : player_list) {
		for (auto property_idx = player.owned_cbegin() ; property_idx < player.owned_cend() ; property_idx++) {
			Space& space = board.at(*property_idx);
			auto& property = std::get<Property>(space);
			fmt::print("{} owns {}\n", player.name, property.name);
		}
	}
	for (Player& player : player_list) {
		fmt::print("{} stats {}\n", player.name, player.get_stats());
	}
	return 0;
}

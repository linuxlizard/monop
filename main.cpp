// davep 20240615 I'm bored. Let's play with C++ and write a monopoly simulator.

// After having played with Rust for a while, can I do C++ without any dynamic memory?
// Can I do C++ without any OOP-y-ness (which I hear has fallen out of fashion)


#include <iostream>
#include <ranges>
#include <vector>
#include <fmt/format.h>
#include <cassert>

#include "board.h"
#include "player.h"


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
					if (player.in_jail() ) {
						fmt::print("{} is no longer in jail\n", player.name);
						player.leave_jail();
						// end of turn; go to next player
						break;
					}
				}

				if (player.in_jail() ) {
					if (player.do_jail_turn()) {
						// player has had three turns in jail
						// so time to leave
						player.leave_jail();
					}

					// end of turn; go to next player
					break;
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
				else if (std::holds_alternative<Railroad>(land)) {
					auto& rr = std::get<Railroad>(land);
					std::optional<uint> owner = get_owner<Railroad>(rr);

					if (owner) {
						auto& owner_player = player_list.at(owner.value());
						fmt::print("{} is owned by {}\n", rr.name, owner_player.name);
						if ( owner != player_idx ) {
							// owner is not self so rent must be paid
//							uint rent = property.get_rent(NO_HOUSES);
							uint rent = rr.get_rent(owner_player.railroads_owned());
							fmt::print("{} must pay {} {} in rent on Railroad(s)\n", player.name, owner_player.name, rent);
							owner_player.earn_rent( player.pay_rent(rent));
						}
					}
					else {
						fmt::print("{} is unowned\n", rr.name);

						if (player.buys(rr)) {
							rr.set_owner(player_idx);
						}
					}
				}


				if (rolled_doubles) {
					fmt::print("{} has rolled doubles and gets another turn\n", player.name);
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

//			std::string name = std::visit([](auto&& arg) -> std::string { return arg.name; }, space);

			std::string name = get_name(space);
//			auto& property = std::get<Property>(space);
			fmt::print("{} owns {}\n", player.name, name);
		}
	}
	for (Player& player : player_list) {
		fmt::print("{} stats {}\n", player.name, player.get_stats());
	}

	for (const Space &space : board ) {
		fmt::print("{}\n", get_name(space));
	}
	return 0;
}

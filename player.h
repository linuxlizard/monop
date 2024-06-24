//
// Created by dpoole on 6/24/24.
//

#ifndef MONOP_PLAYER_H
#define MONOP_PLAYER_H

#include "board.h"
#include "dice.h"

struct PlayerStats
{
	// counters;
	uint moves;
	uint passed_go;
	uint properties_purchased;
	uint money_spent;
	uint doubles;
	uint rent_paid;
	uint rent_earned;
	uint taxes_paid;
};

class Player
{
public:
	explicit Player(std::string name, int cash) :
			name{std::move(name)},
			cash{cash},
			position{0},
			_passed_go{false},
			die1{0},
			die2{0},
			stats{{}}
	{};

	std::pair<uint,uint> roll()
	{
		die1 = roll_die();
		die2 = roll_die();
		if (die1 == die2) {
			stats.doubles += 1;
		}
		return std::make_pair(die1,die2);
	}

	void sanity_check() const
	{
		// sanity check as the game continues
		if (cash < 0) {
			std::string msg = fmt::format("error! player {} has negative cash={}\n", name, cash);
			throw std::runtime_error(msg);
		}
	}

	uint move()
	{
		sanity_check();
		stats.moves += 1;

		position += die1 + die2;

		_passed_go = false;
		if (position >= BOARD_SIZE) {
			_passed_go = true;
			stats.passed_go += 1;
			position = position % BOARD_SIZE;
		}

		return position;
	}

	void add_money(int new_money) { cash += new_money; }

	[[nodiscard]] bool passed_go() const { return _passed_go; }

	const std::string name;

	bool buys(const Property &property);

	std::string get_stats();

	[[nodiscard]] std::vector<uint>::const_iterator owned_cbegin() const {
		return property_owned.cbegin();
	}

	[[nodiscard]] std::vector<uint>::const_iterator owned_cend() const {
		return property_owned.cend();
	}

	uint pay_rent(uint rent);
	void pay_tax(uint tax);
	void go_to_jail();

	void earn_rent(uint rent) {
		add_money(rent);
		stats.rent_earned += rent;
	}

private:
	// note signed integer so can detect negative
	int cash;
	// where I am on the board (index)
	uint position;
	uint die1, die2;
	bool _passed_go;

	std::vector<uint> property_owned;

	struct PlayerStats stats;

};

#endif //MONOP_PLAYER_H

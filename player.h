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
	int go_to_jail;
};

class Player
{
public:
	explicit Player(std::string name, int cash) :
			name{std::move(name)},
			cash{cash},

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

	uint move();

	void add_money(int new_money) { cash += new_money; }

	[[nodiscard]] bool passed_go() const { return _passed_go; }

	const std::string name{};

	bool buys(const Property &property);
	bool buys(const Railroad &rr);

	[[nodiscard]] uint railroads_owned() const { return _railroads_owned; }

	std::string get_stats();

	[[nodiscard]] std::vector<uint>::const_iterator owned_cbegin() const {
		return property_owned.cbegin();
	}

	[[nodiscard]] std::vector<uint>::const_iterator owned_cend() const {
		return property_owned.cend();
	}

	uint pay_rent(uint rent);
	void pay_tax(uint tax);

	void earn_rent(uint rent) {
		add_money(rent);
		stats.rent_earned += rent;
	}

	void go_to_jail();
	[[nodiscard]] bool in_jail() const { return jail_counter > 0; };
	void leave_jail() { jail_counter = 0; };

	[[nodiscard]] bool do_jail_turn()
	{
		// TODO add option for $50 bail
		return ++jail_counter > 3;
	};

private:
	// note signed integer so can detect negative
	int cash {0};
	// where I am on the board (index)
	uint position {0};
	uint die1 {0}, die2 {0};

	bool _passed_go {false};
	uint jail_counter {0};

	std::vector<uint> property_owned {};

	uint _railroads_owned {0};

	struct PlayerStats stats {};

	bool buy_something(const std::string & name, uint price);
};

#endif //MONOP_PLAYER_H

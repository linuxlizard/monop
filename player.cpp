//
// Created by dpoole on 6/24/24.
//

#include <string>
#include <fmt/format.h>

#include "board.h"
#include "dice.h"
#include "player.h"

std::string Player::get_stats() {
	return fmt::format("moves={} passed_go={} jail={} doubles={} purchased={} spent={} rent_paid={} rent_earned={} taxes={} cash={}",
	                   stats.moves, stats.passed_go, stats.go_to_jail, stats.doubles, stats.properties_purchased,
	                   stats.money_spent, stats.rent_paid, stats.rent_earned, stats.taxes_paid, cash);
}

bool Player::buy_something( const std::string& object_name, uint price)
{
	if (price <= cash) {
		// always buy if we have enough money
		// TODO we'll want to raise money to buy valuable properties
		//  (e.g, mortgage something to buy Boardwalk)
		cash -= static_cast<int>(price);

		fmt::print("{} buys {} for {} and has {} remaining\n",
		           name, object_name, price, cash);

		stats.properties_purchased += 1;
		stats.money_spent += price;

		// track owned properties by the board position because
		// the Property argument is inside a std::variant and
		// can't be stored (I think???)

		// sanity check for duplicates:
		assert(std::find(property_owned.begin(), property_owned.end(), position) == property_owned.end());

		property_owned.emplace_back(position);

		return true;
	}

	fmt::print("{} only has {} left and cannot pay {} for {}\n",
	           name, cash, price, object_name );
	return false;
}

bool Player::buys(const Property &property)
{

	sanity_check();
	uint price = property.get_purchase_price();

	return buy_something(property.name, price);
}

bool Player::buys(const Railroad &rr)
{
	sanity_check();
	uint price = static_cast<int>(rr.get_purchase_price());

	bool flag = buy_something(rr.name, price);
	if (flag) {
		_railroads_owned += 1;
	}
	return flag;
}

uint Player::pay_rent(uint rent)
{
	if (rent > cash)
	{
		fmt::print("player {} is bankrupt!\n", name);
		throw std::runtime_error("unimplemented");
	}
	stats.rent_paid += rent;
	stats.money_spent += rent;

	cash -= rent;
	return rent;
}

uint Player::move()
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

void Player::pay_tax(uint tax)
{
	if (tax > cash)
	{
		fmt::print("player {} is bankrupt!\n", name);
		throw std::runtime_error("unimplemented");
	}
	stats.taxes_paid += tax;
	stats.money_spent += tax;

	cash -= tax;
}

void Player::go_to_jail()
{
	fmt::print("player {} goes to jail\n", name);

	assert (jail_counter==0);

	jail_counter = 1;
	stats.go_to_jail += 1;

	// Go to Jail. Go directly to Jail.
	// Do not Pass Go. Do not collection $200.
	position = 30;
}

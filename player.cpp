//
// Created by dpoole on 6/24/24.
//

#include <string>
#include <fmt/format.h>

#include "board.h"
#include "dice.h"
#include "player.h"

std::string Player::get_stats() {
	return fmt::format("moves={} passed_go={} doubles={} purchased={} spent={} rent_paid={} rent_earned={} cash={}",
	                   stats.moves, stats.passed_go, stats.doubles, stats.properties_purchased,
	                   stats.money_spent, stats.rent_paid, stats.rent_earned, cash);
}

bool Player::buys(const Property &property)
{
	sanity_check();

	int price = static_cast<int>(property.get_purchase_price());
	if (price <= cash) {
		// always buy if we have enough money
		// TODO we'll want to raise money to buy valuable properties
		//  (e.g, mortgage something to buy Boardwalk)
		cash -= price;

		fmt::print("{} buys {} for {} and has {} remaining\n",
		           name, property.name, property.get_purchase_price(), cash);

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
	           name, cash, property.get_purchase_price(), property.name );
	return false;
}

uint Player::pay_rent(uint rent)
{
	if (rent > cash)
	{
		fmt::print("player {} is bankrupt!", name);
		throw std::runtime_error("unimplemented");
	}
	stats.rent_paid += rent;
	stats.money_spent += rent;

	cash -= rent;
	return rent;
}

void Player::pay_tax(uint tax)
{
	if (tax > cash)
	{
		fmt::print("player {} is bankrupt!", name);
		throw std::runtime_error("unimplemented");
	}
	stats.taxes_paid += tax;
	stats.money_spent += tax;

	cash -= tax;
}

void Player::go_to_jail()
{

}

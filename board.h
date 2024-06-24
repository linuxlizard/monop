//
// Created by dpoole on 6/24/24.
//

#ifndef MONOP_BOARD_H
#define MONOP_BOARD_H

#include <cassert>
#include <vector>

// spaces on the monopoly board
const uint BOARD_SIZE = 40;

enum Rent
{
	NO_HOUSES,
	ALL_COLORS,  // rent doubled when all color block is owned
	ONE_HOUSE,
	TWO_HOUSES,
	THREE_HOUSES,
	FOUR_HOUSES,
	HOTEL
};

enum PenaltyType
{
	PENALTY_INCOME_TAX,
	PENALTY_LUXURY_TAX,
	PENALTY_GO_TO_JAIL
};

class Card
{
public:
	explicit Card(std::string name) : name{std::move(name)} {};
	explicit Card(const char *name) : name(name) {};

	const std::string name;
};

class GoCard
{
public:
	explicit GoCard(std::string name) : name{std::move(name)} {};

	const std::string name;
};

template<typename T> std::optional<uint> get_owner( T& c)
{
	return c._get_owner();
}

class Property {
public:
	Property() = delete;
	Property(Property&) = delete;

	Property(Property&&) = default;

	explicit Property(std::string name, std::vector<uint> values);

	const std::string name;

	uint get_rent(enum Rent r) { return this->rents.at(r); };
	[[nodiscard]] uint get_purchase_price() const { return this->purchase_price; };

	[[nodiscard]] std::optional<uint> _get_owner () const { return owner; };

	void set_owner(uint player_idx) {
		assert( !owner );
		owner = player_idx;
		// verify owner is now player_idx
		assert(owner);
		assert(_get_owner() == player_idx);

	};

private:
	uint purchase_price;
	uint house_cost;
	std::array<uint,7> rents{};
	std::optional<uint> owner;
};

class Railroad
{
public:
	explicit Railroad(std::string  name) : name(std::move(name)) { };
	const std::string name;
};

class Penalty
{
public:
	explicit Penalty(std::string name, enum PenaltyType penalty_type) : name(std::move(name)), penalty_type(penalty_type) { };
//	explicit Penalty(std::string name) : name(std::move(name)) { };
	const std::string name;
	const enum PenaltyType penalty_type;
};

class CommunityChest
{
public:
	explicit CommunityChest() : name("Community Chest") {};
	const std::string name;
};

class Chance
{
public:
	explicit Chance() : name("Chance") {};
	const std::string name;
};

class Utility
{
public:
	explicit Utility(std::string  name) : name(std::move(name)) {};
	const std::string name;
};

using Space = std::variant<GoCard, Property, CommunityChest, Chance, Penalty, Railroad, Utility>;

#endif //MONOP_BOARD_H

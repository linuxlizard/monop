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

template<typename T> std::optional<uint> get_owner( T& c)
{
	return c._get_owner();
}

class Card
{
public:
	Card(std::string name)
		: name{std::move(name)}
	{}

	const std::string name{};
};

class GoCard : public Card
{

};

class Property : public Card
{
public:
	// let's make sure we know how to properly move (no copies)
	Property() = delete;
	Property(Property&) = delete;

	Property(Property&&) = default;

	explicit Property(std::string name, std::vector<uint> values);

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
	uint purchase_price {0};
	uint house_cost {0};
	std::array<uint,7> rents{};
	std::optional<uint> owner {};
};

class Railroad : public Card
{
public:
	explicit Railroad(std::string name)
		: Card(std::move(name)), owner {}
	{}

	static constexpr std::array<uint,5> rr_rent { 0, 25, 50, 100, 200 };

	[[nodiscard]] std::optional<uint> _get_owner () const { return owner; }
	[[nodiscard]] static uint get_purchase_price()  { return 200; }

	[[nodiscard]] static uint get_rent(uint count)
	{
		return rr_rent.at(count);
	}

	void set_owner(uint player_idx) {
		assert( !owner );
		owner = player_idx;
		// verify owner is now player_idx
		assert(owner);
		assert(_get_owner() == player_idx);
	}

private:
	std::optional<uint> owner{};
};

class Penalty : public Card
{
public:
	explicit Penalty(std::string name, enum PenaltyType penalty_type)
			: Card(std::move(name)), penalty_type(penalty_type)
			{ };

	const enum PenaltyType penalty_type;
};

class CommunityChest : public Card
{

};

class Chance : public Card
{

};

class Utility : public Card
{

};

using Space = std::variant<GoCard, Property, CommunityChest, Chance, Penalty, Railroad, Utility>;

std::vector<Space> load_board(const std::string &file_name);

std::string get_name(const Space& space);

#endif //MONOP_BOARD_H

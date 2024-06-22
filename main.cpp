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

// spaces on the monopoly board
const uint BOARD_SIZE = 40;

// clion AI Assistant
// FIXME probably don't need to initialize all these every time
int roll_die()
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> distr(1, 6);
	return distr(gen);
}

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

template<typename Ownable>
struct Owner : Ownable
{
	explicit Owner(Ownable const& ownable) : Ownable(ownable) {}
	
	[[nodiscard]] std::optional<uint> get_owner() const
	{
		return this->_get_owner();
	}
};

class Property {
public:
	Property() = delete;

	explicit Property(std::string name, std::vector<uint> values);

	const std::string name;

	uint get_rent(enum Rent r) { return this->rents.at(r); };
	[[nodiscard]] uint get_purchase_price() const { return this->purchase_price; };

	[[nodiscard]] std::optional<uint> _get_owner () const { return owner; };

	void set_owner(uint player_idx) { owner = player_idx; };

private:
	uint purchase_price;
	uint house_cost;
	std::array<uint,7> rents{};
	std::optional<uint> owner;
};

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

class Railroad
{
public:
	explicit Railroad(std::string  name) : name(std::move(name)) { };
	const std::string name;
};

class Penalty
{
public:
	explicit Penalty(std::string  name) : name(std::move(name)) { };
	const std::string name;
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

struct PlayerStats
{
	// counters;
	uint passed_go;
	uint properties_purchased;
	uint money_spent;
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

	std::pair<uint,uint> roll() {
		die1 = roll_die();
		die2 = roll_die();
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

	std::vector<uint> get_owned_list();

	[[nodiscard]] std::vector<uint>::const_iterator owned_cbegin() const {
		return property_owned.cbegin();
	}

	[[nodiscard]] std::vector<uint>::const_iterator owned_cend() const {
		return property_owned.cend();
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

std::string Player::get_stats() {
	return fmt::format("passed_go={} purchased={} spent={} cash={}",
					   stats.passed_go, stats.properties_purchased, stats.money_spent, cash);
}

bool Player::buys(const Property &property)
{
	sanity_check();

	int price = static_cast<int>(property.get_purchase_price());
	if (price <= cash) {
		// always buy if we have enough money
		cash -= price;

		fmt::print("{} buys {} for {} and has {} remaining\n",
		           name, property.name, property.get_purchase_price(), cash);

		stats.properties_purchased += 1;
		stats.money_spent += price;

		// track owned properties by the board position
		// the Property argument is inside a std::variant and
		// can't be stored (???)
		property_owned.emplace_back(position);

		return true;
	}

	fmt::print("{} only has {} left and cannot pay {} for {}\n",
	           name, cash, property.get_purchase_price(), property.name );
	return false;
}

std::vector<uint> Player::get_owned_list()
{
	// return a copy
	// TODO I bet there's a cleaner way to do this
	return property_owned;
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
			fmt::print(stderr, "property {} has invalid argument {}\n", tokens[0], err.what());
		}

		Property p(tokens[0], values);

		//fmt::print("{}\n", std::get<Property>(board.back()).name);
		fmt::print("{} price={} hotel rent={}\n", p.name, p.get_purchase_price(), p.get_rent(HOTEL));
		return p;
	}

	if (tokens[0] == "Community Chest") {
		return CommunityChest();
	}
	if (tokens[0] == "Chance") {
		return Chance();
	}
	if (tokens[0] == "Income Tax") {
		return Penalty(tokens[0]);
	}

	fmt::print("railroad? {} {}\n", tokens[0].at(tokens[0].size() - 1), tokens[0].at(tokens[0].size() - 2));

	if (tokens[0].at(tokens[0].size() - 1) == 'R' && tokens[0].at(tokens[0].size() - 2) == 'R') {
		return Railroad(tokens[0]);
	}

	if (tokens[0] == "Jail" || tokens[0] == "Go To Jail" || tokens[0] == "Luxury Tax") {
		return Penalty(tokens[0]);
	}

	if (tokens[0] == "Electric Company" || tokens[0] == "Water Works") {
		return Utility(tokens[0]);
	}

	if (tokens[0] == "Free Parking") {
		return Utility(tokens[0]);
	}

	if (tokens[0] == "Go") {
		return GoCard(tokens[0]);
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
		board.emplace_back(parse_board_line(line));

		auto last = board.back();
		if (const Property *p = std::get_if<Property>(&last) ) {
			fmt::print("add {} to the board\n", p->name);
		}
	}

	return board;
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

	for (uint i=0 ; i<20 ; i++ ) {
		for ( uint player_idx=0 ; player_idx<player_list.size() ; player_idx++ ) {

			while (true) {
				Player &player = player_list.at(player_idx);
				auto roll = player.roll();
				fmt::print("Player {} rolled {} {}\n", player.name, roll.first, roll.second);
				uint pos = player.move();

				auto land = board.at(pos);
				std::visit([&player](auto &&arg) { fmt::print("player {} landed on {}\n", player.name, arg.name); }, land);

				if (player.passed_go()) {
					fmt::print("player {} has passed Go and earns 200!\n", player.name);
					player.add_money(200);
				}

				if (std::holds_alternative<Property>(land)) {
					Property & property = std::get<Property>(land);

					Owner o(property);

					if (!o.get_owner()) {
						fmt::print("{} is unowned\n", property.name);

						if (player.buys(property)) {
							property.set_owner(player_idx);
						}
					}
				}

				bool rolled_doubles = roll.first == roll.second;
				if (rolled_doubles) {
					// TODO goto jail on rolling three doubles
					fmt::print("{} rolled doubles!\n", player.name);
					continue;
				}

				fmt::print("{} has completed their turn\n", player.name);
				break;
			};
		}
	}

	for (Player& player : player_list) {
		std::vector<uint> owned { player.get_owned_list() };
		for (auto b = player.owned_cbegin() ; b < player.owned_cend() ; b++) {
			auto space = board.at(*b);
			Property & property = std::get<Property>(space);
			fmt::print("{} owns {}\n", player.name, property.name);
		}

		fmt::print("{} stats {}\n", player.name, player.get_stats());
	}

	return 0;
}

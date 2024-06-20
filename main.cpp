// davep 20240615 I'm bored. Let's play with C++ and write a monopoly simulator.

// After having played with Rust for a while, can I do C++ without any dynamic memory?


#include <iostream>
#include <fstream>
#include <random>
#include <cstring>
#include <cerrno>
#include <sstream>
#include <ranges>
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

class Player
{
public:
	explicit Player(uint cash) : cash{cash}, position{0}, _passed_go{false} {};

	std::pair<uint,uint> roll() {
		die1 = roll_die();
		die2 = roll_die();
		return std::make_pair(die1,die2);
	}

	uint move() {
		position += die1 + die2;

		if (position > BOARD_SIZE) {
			_passed_go = true;
			position = position % BOARD_SIZE;
		}
		else {
			_passed_go = false;
		}

		return position;
	}

	void add_money(uint moola) { cash += moola; }

	uint where() { return position; }

	bool passed_go() {
		return _passed_go;
	}

	[[nodiscard]] uint getCash() const {
		return cash;
	}

private:
	uint cash;

	// where I am on the board (index)
	uint position;

	uint die1, die2;

	bool _passed_go;
};

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

class Property {
public:
	Property() = delete;

	explicit Property(std::string  name, std::vector<uint> values);

	const std::string name;

	uint get_rent(enum Rent r) { return this->rents.at(r); };
	[[nodiscard]] uint get_purchase_price() const { return this->purchase_price; };

private:
	uint purchase_price;
	uint house_cost;
	std::array<uint,7> rents{};
};

Property::Property(std::string  name, std::vector<uint> values)
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

Space parse_board_line(const std::string &line) {
	// https://www.youtube.com/watch?v=V14xGZAyVKI
	auto split_strings = std::ranges::views::split(line, ',');

	std::vector<std::string> tokens;

	for (const auto &rng: split_strings) {

		tokens.emplace_back(rng.begin(), rng.end());
	}

	fmt::print("size={}\n", tokens.size());

	tokens.clear();
	std::ranges::copy(split_strings | std::ranges::views::transform([](auto &&rng) {
		std::string s(rng.begin(), rng.end());
		return s;
	}), std::back_inserter(tokens));
	fmt::print("found len={} tokens. First={}\n", tokens.size(), tokens[0]);

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

#if 0
struct GetName
{
	void get_name()(const Property& p) const { fmt::print("name={}\n", p.name); };

};
#endif

int main() {
	std::cout << "Hello, World!" << std::endl;

	const std::string board_filename { "/home/dpoole/src/monop/board.txt" };
	std::vector<Space> board = load_board(board_filename);

	assert(board.size() == BOARD_SIZE && "The board must contain 40 elements.");

	Player p1 { 1500 };
	Player p2 { 1500 };

	for (int i=0 ; i<10 ; i++ ) {
		auto roll = p1.roll();

		fmt::print("You rolled {} {}\n", roll.first, roll.second);

		uint pos = p1.move();

		auto land = board.at(pos);

		//std::visit(GetName{}, land);
		std::visit([](auto&& arg){ fmt::print("{}\n",arg.name); }, land);

		if (p1.passed_go()) {
			fmt::print("player has passed Go!\n");
			p1.add_money(200);
		}

	}

	return 0;
}

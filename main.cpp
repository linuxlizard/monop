// davep 20240615 I'm bored. Let's play with C++ and write a monopoly simulator.

#include <iostream>
#include <fstream>
#include <random>
#include <cstring>
#include <cerrno>
#include <sstream>
#include <ranges>
#include <string_view>
#include <fmt/format.h>

class Player
{
public:
	explicit Player(uint cash) : cash{cash} {};

public:
	[[nodiscard]] uint getCash() const {
		return cash;
	}
private:
	uint cash;
};

class Property {
public:
	Property(uint price, const char * name) :
		price{price}, name{name} {};

	std::string name;

private:
	uint price;
};

// community chest
// or chance
class RandomCard
{
public:
	RandomCard( );
};

RandomCard::RandomCard() = default;

class Penalty
{

};

using Space = std::variant<Property, RandomCard, Penalty>;

// clion AI Assistant
int roll_die() {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> distr(1, 6);
	return distr(gen);
}

void parse_board_line(const std::string &line)
{
	// https://www.youtube.com/watch?v=V14xGZAyVKI
	auto split_strings = std::ranges::views::split(line, ',');

	std::vector<std::string> tokens;

	for (const auto &rng: split_strings) {

		tokens.emplace_back(rng.begin(), rng.end());
	}

	fmt::print("size={}\n", tokens.size());

	tokens.clear();
	std::ranges::copy( split_strings | std::ranges::views::transform([](auto &&rng) {
		std::string s(rng.begin(), rng.end());
		return s;
	}), std::back_inserter(tokens));
	fmt::print("found len={} tokens\n", tokens.size());

#if 0
	std::vector<Space> spaces(split_strings | std::ranges::views::transform([](auto &&rng) {
		std::string_view sv(rng.begin(), rng.end());
		if (sv == "Property") {
			return Space(Property{});
		} else if (sv == "RandomCard") {
			return Space(RandomCard{});
		} else if (sv == "Penalty") {
			return Space(Penalty{});
		} else {
			throw std::runtime_error("Invalid space type");
		}
	}));
#endif
}

void parse_board_line2(const std::string &line)
{
	std::istringstream v { line };
	std::vector<std::string> tokens;
	std::string token;
	while (std::getline(v, token, ',')) {
		tokens.push_back(token);
	}
	std::cout << "found " << tokens.size() << " tokens\n";
}

int load_board(const std::string &file_name) {
	std::ifstream input_file(file_name);

	if (input_file.fail()) {
		std::cerr << "failed to open " << file_name << " " << std::strerror(errno) << "\n";
		return -1;
	}

	std::string line;
	while (std::getline(input_file, line)) {
		std::cout << line.length() << "\n";
		parse_board_line(line);
	}

	return 0;
}

int main() {
	std::cout << "Hello, World!" << std::endl;

	const std::string board_filename { "/home/dpoole/src/monop/board.txt" };
	load_board(board_filename);

	int die1 = roll_die();
	int die2 = roll_die();
	std::cout << "You rolled a " << die1 << " and a " << die2 << ".\n";

	Player p1 { 1500 };
	Player p2 { 1500 };

	std::vector<Space> board;
	board.emplace_back( Property(60, "Mediterranean Avenue") );
//	board.emplace_back( CommunityChest() );
	board.emplace_back( Property(60, "Baltic Avenue") );


	return 0;
}

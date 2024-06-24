//
// Created by dpoole on 6/24/24.
//

#include <random>

#include "dice.h"

// clion AI Assistant
// FIXME probably don't need to initialize all these every time
int roll_die()
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> distr(1, 6);
	return distr(gen);
}

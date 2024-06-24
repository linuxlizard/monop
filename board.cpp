//
// Created by dpoole on 6/24/24.
//
#include <string>
#include <vector>
#include <fmt/format.h>

#include "board.h"

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

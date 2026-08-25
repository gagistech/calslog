/*
calslog - Calories logging mobile applcation

Copyright (C) 2026-2026 Gagistech Oy <gagisechoy@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

/* ================ LICENSE END ================ */

#pragma once

#include <chrono>
#include <limits>
#include <map>
#include <vector>

#include <fsif/file.hpp>

namespace calslog::model {

struct entry {
	std::u32string name;
	uint32_t pcs; // number of pieces
	float mass; // mass of 1 piece in grams
	float kcal; // per 100 grams
};

struct day {
	std::chrono::year_month_day date;
	std::vector<entry> entries;
};

struct food {
	std::u32string name;
	float kcal; // per 100 grams
	float mass; // mass of 1 piece in grams
};

struct root {
	std::vector<day> history;

	day today;

	std::vector<food> foods;
};

root read(const fsif::file& fi);
void write(const root& r, fsif::file& fi);

} // namespace calslog::model

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
#include <utki/signal.hpp>

namespace calslog::model {

struct entry {
	std::u32string name;
	uint32_t pcs; // number of pieces
	uint32_t mass; // mass of 1 piece in grams
	uint32_t kcal; // per 100 grams
	bool enabled = true; // if false, the entry is not counted in the day total

	uint32_t calc_total_kcal() const
	{
		return this->kcal * this->mass * this->pcs / 100;
	}
};

struct day {
	std::chrono::year_month_day date;
	std::vector<entry> entries;
	uint32_t weight = 0; // weight in grams, 0 means the weight was not logged for that day

	uint32_t calc_total_kcal() const
	{
		uint32_t total = 0;
		for (const auto& entry : this->entries) {
			if (entry.enabled) {
				total += entry.calc_total_kcal();
			}
		}
		return total;
	}

	// Formats the weight in kilograms with up to one digit after the decimal point
	// (e.g. "1.2"). Returns "?" if the weight was not logged (0 grams).
	std::string get_weight_string() const;
};

struct food {
	std::u32string name;
	uint32_t kcal; // per 100 grams
	uint32_t mass; // mass of 1 piece in grams
};

struct root {
	std::vector<day> history;

	day today;

	std::vector<food> foods;

	// Emitted whenever the model data is modified, so that interested parties
	// (e.g. the GUI) can react by refreshing their state.
	utki::signal<> model_changed_signal;
};

root read(const fsif::file& fi);
void write(const root& r, fsif::file& fi);

} // namespace calslog::model

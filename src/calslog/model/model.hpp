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
#include <cmath>
#include <limits>
#include <map>
#include <vector>

#include <fsif/file.hpp>
#include <utki/signal.hpp>

namespace calslog::model {

// The default calorie goal for a day, used when no goal has been set yet.
constexpr uint32_t default_day_goal_kcal = 2000;

struct entry {
	std::u32string name;
	float pcs; // number of pieces (may be fractional, e.g. 0.5 or 0.25)
	uint32_t mass; // mass of 1 piece in grams
	uint32_t kcal; // per 100 grams
	bool enabled = true; // if false, the entry is not counted in the day total

	uint32_t calc_total_kcal() const
	{
		// The number of pieces may be fractional, so the calculation is done with
		// floating point arithmetic and the result is rounded to the nearest
		// integer, since fractions of kcal are not representative.
		return uint32_t(std::round(this->kcal * this->mass * this->pcs / 100.f));
	}
};

struct day {
	std::chrono::year_month_day date;
	std::vector<entry> entries;
	uint32_t weight = 0; // weight in grams, 0 means the weight was not logged for that day
	uint32_t day_goal_kcal = 0; // calorie goal for this day, 0 means no goal was set

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

	// Returns the calorie goal to prefill for the next day: this day's goal if it
	// is set, otherwise the default goal.
	uint32_t next_day_goal() const
	{
		return this->day_goal_kcal != 0 ? this->day_goal_kcal : default_day_goal_kcal;
	}
};

struct food {
	std::u32string name;
	uint32_t kcal; // per 100 grams
	uint32_t mass; // mass of 1 piece in grams
};

struct root {
	std::vector<day> history;

	std::vector<food> foods;

	// Emitted whenever the model data is modified, so that interested parties
	// (e.g. the GUI) can react by refreshing their state.
	utki::signal<> model_changed_signal;

	// Returns a reference to the most recent (last) day in the history. New entries
	// are always logged to this day. The application guarantees at start-up that this
	// day corresponds to today's date, pushing a fresh (empty) day when it does not.
	day& today()
	{
		return this->history.back();
	}

	const day& today() const
	{
		return this->history.back();
	}
};

root read(const fsif::file& fi);
void write(const root& r, fsif::file& fi);

} // namespace calslog::model

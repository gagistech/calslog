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

#include "util.hpp"

namespace calslog {
std::chrono::year_month_day log_date_for(
	std::chrono::year_month_day calendar_date, //
	uint32_t minutes_since_midnight, //
	uint32_t day_flip_minutes
)
{
	// The calendar date on which the current 24-hour log period begins.
	const std::chrono::sys_days base{calendar_date};
	const auto period_start_date = //
		(minutes_since_midnight >= day_flip_minutes) ? base : base - std::chrono::days{1};

	// If the flip is at/after noon the period belongs to the day after its start.
	const auto offset = (day_flip_minutes < 720) ? std::chrono::days{0} : std::chrono::days{1};

	return std::chrono::year_month_day{period_start_date + offset};
}

std::chrono::year_month_day current_log_date(uint32_t day_flip_minutes)
{
	const auto now = std::chrono::system_clock::now();
	const auto& tz = std::chrono::current_zone();
	const auto lt = std::chrono::zoned_time(tz, now).get_local_time();
	const auto ld = std::chrono::floor<std::chrono::days>(lt);
	const auto minutes_since_midnight = std::chrono::duration_cast<std::chrono::minutes>(lt - ld).count();

	return log_date_for(
		std::chrono::year_month_day{ld}, //
		static_cast<uint32_t>(minutes_since_midnight), //
		day_flip_minutes //
	);
}
} // namespace calslog

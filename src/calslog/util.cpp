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

#include <ctime>

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
	// std::chrono::current_zone() (the C++20 chrono time zone API) is not
	// implemented in the Android NDK libc++, so convert the current UTC time
	// point to local time with localtime_r() instead.
	const auto now = std::chrono::system_clock::now();
	const std::time_t tt = std::chrono::system_clock::to_time_t(now);

	std::tm lt{};
	localtime_r(&tt, &lt);

	const auto minutes_since_midnight = //
		static_cast<uint32_t>(lt.tm_hour) * 60u + static_cast<uint32_t>(lt.tm_min);

	return log_date_for(
		std::chrono::year{lt.tm_year + 1900} / //
		static_cast<std::chrono::month>(lt.tm_mon + 1) / //
		lt.tm_mday, //
		minutes_since_midnight, //
		day_flip_minutes //
	);
}
} // namespace calslog

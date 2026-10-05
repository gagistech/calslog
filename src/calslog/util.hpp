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
#include <cstdint>

namespace calslog {

// Computes the food-log "date" for a given moment.
//
// A log "day" is the 24-hour period [D @ flip, (D+1) @ flip) for some calendar
// date D. It is labelled by the calendar date that covers most of that period,
// so the flip always falls within the first 12 hours of the day it labels.
// Given the current calendar date and the number of minutes since local
// midnight this means:
//  - flip in the first half of the day (00:00-12:00): before the flip the log
//    date is the previous calendar date, after the flip the current one;
//  - flip in the second half of the day (12:00-24:00): before the flip the log
//    date is the current calendar date, after the flip already the next one.
std::chrono::year_month_day log_date_for(
	std::chrono::year_month_day calendar_date, //
	uint32_t minutes_since_midnight, //
	uint32_t day_flip_minutes
);

// Returns the current food-log "date" using the system clock and local time zone.
std::chrono::year_month_day current_log_date(uint32_t day_flip_minutes);

} // namespace calslog

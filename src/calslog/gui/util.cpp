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

#include <format>
#include <string>

#include <utki/string.hpp>
#include <utki/unicode.hpp>

namespace calslog {

float to_float(const std::u32string& s)
{
	auto s8 = utki::to_utf8(s);
	utki::string_parser p(s8);
	return p.read_number<float>();
}

std::u32string make_date_string(std::chrono::year_month_day date)
{
	return utki::to_utf32(std::format(
		"{:04d}-{:02d}-{:02d}", //
		static_cast<int>(date.year()), //
		static_cast<unsigned>(date.month()), //
		static_cast<unsigned>(date.day())
	));
}

kcal_wording make_kcal_wording(
	const ruis::localization& loc, //
	const model::day& day
)
{
	using namespace std::string_view_literals;

	const auto total_kcal = day.calc_total_kcal();
	const auto total = utki::to_utf32(std::to_string(total_kcal));
	if (day.day_goal_kcal != 0) {
		return {
			.total = total, //
			.goal = loc.get("kcal_goal_suffix"sv).format({utki::to_utf32(std::to_string(day.day_goal_kcal))}), //
			.exceeded = total_kcal > day.day_goal_kcal
		};
	}
	return {
		.total = loc.get("kcal"sv).format({total}), //
		.goal = std::u32string{},
		.exceeded = false
	};
}

} // namespace calslog

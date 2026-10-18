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

#include "day_flip_time.hpp"

#include <chrono>
#include <format>
#include <ratio>
#include <stdexcept>

#include <utki/string.hpp>
#include <utki/unicode.hpp>

namespace calslog {
namespace {
constexpr auto hours_per_day = 24;
constexpr auto minutes_per_hour =
	std::chrono::duration_cast<std::chrono::minutes>(std::chrono::hours{1}).count(); // 60
constexpr auto max_hours = hours_per_day - 1;
constexpr auto max_minutes = minutes_per_hour - 1;
constexpr auto max_deci_minutes = uint32_t(max_minutes / std::deci::den); // 5
constexpr auto digit_base = 10;
} // namespace

std::u32string format_day_flip_time(uint32_t minutes)
{
	return utki::to_utf32(std::format(
		"{:d}:{:02d}", //
		minutes / minutes_per_hour, //
		minutes % minutes_per_hour
	));
}

uint32_t parse_day_flip_time(std::string_view str)
{
	try {
		utki::string_parser p(str);
		p.skip_whitespaces();

		auto hours_view_size = p.get_view().size();
		auto hours = p.read_number<uint32_t>();
		auto hours_digits = hours_view_size - p.get_view().size();

		// The separator is either ':' or '.'
		// (peek_char() throws if the string has ended)
		auto sep = p.peek_char();
		if (sep != ':' && sep != '.') {
			throw std::invalid_argument("invalid time of day");
		}
		p.read_char();

		auto minutes_view_size = p.get_view().size();
		auto minutes = p.read_number<uint32_t>();
		auto minutes_digits = minutes_view_size - p.get_view().size();

		p.skip_whitespaces();

		if (hours_digits > 2 || minutes_digits != 2 || hours > max_hours || minutes > max_minutes ||
			!p.get_view().empty())
		{
			throw std::invalid_argument("invalid time of day");
		}
		return hours * minutes_per_hour + minutes;
	} catch (...) {
		// std::string_parser also throws std::invalid_argument on malformed input
		throw std::invalid_argument("invalid time of day");
	}
}

bool is_valid_day_flip_time(std::string_view str)
{
	try {
		parse_day_flip_time(str);
	} catch (const std::invalid_argument&) {
		return false;
	}
	return true;
}

bool is_day_flip_time_prefix(std::u32string_view str)
{
	size_t i = 0;

	// The hours part: up to 2 digits.
	uint32_t hours = 0;
	size_t hours_digits = 0;
	while (i < str.size()) {
		const auto c = str[i];
		if (c < '0' || c > '9') {
			break;
		}
		hours = hours * digit_base + (c - '0');
		++hours_digits;
		if (hours_digits > 2) {
			return false;
		}
		++i;
	}

	if (hours_digits == 2 && hours > max_hours) {
		// Cannot be extended to a valid hours value.
		return false;
	}

	if (i >= str.size()) {
		// No separator yet.
		return true;
	}

	if (str[i] != ':' && str[i] != '.') {
		return false;
	}
	if (hours_digits == 0) {
		// The separator cannot be the first character.
		return false;
	}
	++i;

	// The minutes part: up to 2 digits.
	uint32_t minutes = 0;
	size_t minutes_digits = 0;
	while (i < str.size()) {
		const auto c = str[i];
		if (c < '0' || c > '9') {
			return false;
		}
		minutes = minutes * digit_base + (c - '0');
		++minutes_digits;
		if (minutes_digits > 2) {
			return false;
		}
		++i;
	}

	if (minutes_digits == 2) {
		return minutes <= max_minutes;
	}
	if (minutes_digits == 1) {
		// A one-digit prefix can only be extended to 00-59 if it is 0-5.
		return minutes <= max_deci_minutes;
	}
	return true; // "H:" or "HH:" can still be completed.
}

bool day_flip_time_input_filter(
	std::u32string_view original, //
	size_t replace_start, //
	size_t replace_end, //
	std::u32string_view to_insert
)
{
	auto result = std::u32string(original.substr(0, replace_start));
	result.append(to_insert);
	result.append(original.substr(replace_end));
	return is_day_flip_time_prefix(result);
}
} // namespace calslog

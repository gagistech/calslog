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

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace calslog {

//
// Day flip time text representation.
//
// The day flip time (minutes after midnight) is entered by the user in the
// settings dialog as an "H:MM" or "H.MM" string (e.g. "3:00" or "3.00"),
// and is stored in the settings model as minutes after midnight. These
// functions convert between the two representations.
//

// Formats the time of day, given as minutes after midnight, as "H:MM".
std::u32string format_day_flip_time(uint32_t minutes);

// Parses the time of day, given as an "H:MM"/"HH:MM" or "H.MM"/"HH.MM" string,
// and returns it as minutes after midnight.
// The hours part must consist of 1 or 2 digits (0-23), the separator can be
// either ':' or '.', and the minutes part must consist of exactly 2 digits
// (00-59), e.g. "3:00", "3.00" or "23:59".
// Throws std::invalid_argument if the input is not a valid time of day.
uint32_t parse_day_flip_time(std::string_view str);

// Returns true if the given string is a valid time of day string.
bool is_valid_day_flip_time(std::string_view str);

// Returns true if the given string is a prefix of a valid day flip time string,
// i.e. if it can be extended to a string in the "H:MM"/"HH:MM" format
// (or the same with '.' instead of ':') with hours 0-23 and minutes 00-59.
bool is_day_flip_time_prefix(std::u32string_view str);

// Input filter for the day flip time text field.
// Rejects an input edit if the resulting text is not a prefix of a valid time string.
// The arguments are the original (current) text, the range [replace_start, replace_end)
// of the original text to be replaced, and the text to be inserted in place of that range.
bool day_flip_time_input_filter(
	std::u32string_view original, //
	size_t replace_start, //
	size_t replace_end, //
	std::u32string_view to_insert
);

} // namespace calslog

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

#include <tml/tree.hpp>
#include <utki/signal.hpp>

namespace calslog {

// The user-configurable application settings.
struct settings_model {
	// The time of day (minutes after midnight) at which the food log "day"
	// flips over to the next day. 3:00 in the morning is a good default.
	constexpr static const auto default_day_flip_minutes = 3 * 60;
	uint32_t day_flip_minutes = default_day_flip_minutes;

	// Mapping of language ids to the native language names
	// used by the UI language selection box.
	static const std::array<std::pair<std::string_view, std::u32string_view>, 3> language_id_to_name_mapping;

	// Index of the currently selected UI language in language_id_to_name_mapping.
	size_t cur_language_index = 0;
};

// Storage for the application settings.
// Reads the settings from a file in the given config directory on construction
// and writes them back to the same file whenever they are changed via set().
class settings
{
	std::string filename;

	settings_model settings_v;

	static settings_model read(std::string_view filename);
	void write();

public:
	utki::signal<const settings_model&> settings_changed_signal;

	explicit settings(std::string_view config_dir);

	const settings_model& get() const noexcept
	{
		return this->settings_v;
	}

	void set(const settings_model& s);
};

} // namespace calslog

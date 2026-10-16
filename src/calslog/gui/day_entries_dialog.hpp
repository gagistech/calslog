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

#include <ruis/widget/widget.hpp>

namespace calslog {

// Shows a modal dialog with the entries logged on the given day.
// The dialog shows the day's date as its title, the day's total calories together
// with the day goal and the logged weight, and a scrollable list of the day's
// entries laid out the same way as the entries on the "today" page (but without
// the interactive controls of those).
// Does nothing if the given day is not present in the model's history.
void show_day_entries_dialog(ruis::widget& owner_widget, std::chrono::year_month_day date);

} // namespace calslog

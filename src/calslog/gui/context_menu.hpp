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

#include <functional>

#include <ruis/widget/widget.hpp>

namespace calslog {

// Shows a context menu with two common items, "Edit" (index 0) and "Delete"
// (index 1), appearing near the given anchor widget. The `on_edit` callback is
// invoked when the "Edit" item is clicked and the `on_delete` callback is
// invoked when the "Delete" item is clicked, right before the menu closes.
// This is the common implementation of the item context menu shared by both the
// foods page and the today page, so that the menu items and their look are
// defined in one place.
void show_item_context_menu(
	ruis::widget& anchor, //
	std::function<void()> on_edit, //
	std::function<void()> on_delete
);

} // namespace calslog

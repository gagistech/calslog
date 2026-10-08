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
#include <vector>

#include <ruis/res/image.hpp>
#include <ruis/style/styled.hpp>
#include <ruis/util/localization.hpp>
#include <ruis/widget/widget.hpp>

namespace calslog {

// Describes a single context menu item: the icon to show on the left (a shared
// ruis image resource, e.g. the "edit" or "delete" icons) and the localized
// wording to show. The wording is a live localization reference (not a string
// snapshot) so that it is re-resolved against the current localization on
// reload (e.g. after a language change).
struct context_menu_item {
	ruis::styled<ruis::res::image> icon;
	ruis::wording wording;
};

// Shows a context menu near the given anchor widget with the given items. Each
// item is a row with its icon on the left and its localized wording on the
// right. The `on_item_click` callback is invoked with the clicked item index,
// right before the menu closes. All context menus in the app (the item menu and
// the top-bar menu) share this single implementation, so the menu items' look
// is defined in one place.
void show_context_menu(
	ruis::widget& anchor, //
	std::vector<context_menu_item> items, //
	std::function<void(size_t index)> on_item_click
);

// Shows a context menu with two common items, "Edit" (index 0) and "Delete"
// (index 1), appearing near the given anchor widget. The `on_edit` callback is
// invoked when the "Edit" item is clicked and the `on_delete` callback is
// invoked when the "Delete" item is clicked, right before the menu closes.
// This is the common implementation of the item context menu shared by both the
// foods page and the today page.
void show_item_context_menu(
	ruis::widget& anchor, //
	std::function<void()> on_edit, //
	std::function<void()> on_delete
);

} // namespace calslog

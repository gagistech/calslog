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

#include <ruis/style/styled.hpp>
#include <ruis/util/length.hpp>
#include <ruis/util/sides.hpp>
#include <ruis/widget/widget.hpp>

namespace calslog {

/**
 * @brief Create the root widget of the main window.
 * @param c - ruis context.
 * @param insets - the live content insets of the window (see
 *        ruisapp::window::get_insets()). The top / bottom insets push the top
 *        bar and the tab bar away from the system status bar and navigation
 *        bar when the window draws edge-to-edge; they are live, so a window
 *        resize (e.g. rotation) is reflected without re-querying.
 */
utki::shared_ref<ruis::widget> make_root_widget(
	const utki::shared_ref<ruis::context>& c, //
	const ruis::sides<ruis::styled<ruis::length>>& insets
);

} // namespace calslog

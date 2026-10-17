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
#include <string>

#include <ruis/util/localization.hpp>
#include <ruis/util/widget_list.hpp>
#include <ruis/widget/group/touch/scroll_area.hpp>

#include "../model/model.hpp"

namespace calslog {

// Converts a numeric UTF-32 string to a float.
// Empty or unparsable input yields 0.
float to_float(const std::u32string& s);

// Formats a date as a UTF-32 string in the "YYYY-MM-DD" form.
std::u32string make_date_string(std::chrono::year_month_day date);

// A day's total calories formatted for display as a pair of texts:
// - `total`: the logged total - just the number (e.g. "1234") when a goal is
//   set, or the number with the unit (e.g. "1234 kcal") when it is not;
// - `goal`: the goal suffix in the form "/XXX kcal", empty when no goal is set;
// - `exceeded`: true when a goal is set and the total exceeds it.
// The `total` text should be displayed with the style's color_critical when
// `exceeded` is true, otherwise with the usual text color; the `goal` text is
// always displayed with the primary text color. The texts are wordings (where
// applicable) so that a text label holding them is re-localized on language
// switch, with the numeric arguments preserved.
struct kcal_wording {
	ruis::string total;
	ruis::string goal;
	bool exceeded = false;
};

kcal_wording make_kcal_wording(
	const ruis::localization& loc, //
	const model::day& day
);

// Creates a scroll area holding the given widgets laid out in a column, to be used as the
// contents of a modal dialog (see ruis::touch::dialog).
// The dialog panel is sized to fit its contents, so a scroll area which does not fit on the
// screen would make the dialog overflow the screen. This scroll area requests as much space as
// its contents need, but the layouts clamp it to the space actually available, so the dialog
// always fits on the screen, is resized as the available space changes and its contents are
// scrolled when they do not fit.
utki::shared_ref<ruis::touch::scroll_area> make_dialog_scroll_area(
	const utki::shared_ref<ruis::context>& c, //
	ruis::widget_list contents
);

} // namespace calslog

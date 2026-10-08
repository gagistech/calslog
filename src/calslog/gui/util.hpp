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

#include <ruis/util/localization.hpp>

#include "../model/model.hpp"

namespace calslog {

// Formats a day's total calories for display: as "XXX/YYY" (the logged amount
// vs. the day goal) when a goal is set, or as just "XXX" when it is not.
// Returns a (formatted) wording rather than a plain string so that a text label
// holding it is re-localized on language switch, with the numeric arguments
// preserved.
ruis::wording make_kcal_wording(const ruis::localization& loc, const model::day& day);

} // namespace calslog

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
#include <limits>

#include <ruis/widget/widget.hpp>

namespace calslog {

// Opens the food edit dialog.
// When `food_index` is the default (the maximum value of size_t), the dialog opens
// empty, its title reads "Add Food" and its primary button reads "Add"; submitting
// appends a new food to the model's foods list. When `food_index` is a valid index
// into `model.foods`, the dialog opens prefilled with that food's current name,
// kcal/100g and mass per serving, its title reads "Edit Food" and its primary button
// reads "Save"; submitting updates that food in place.
void show_food_edit_dialog(
	ruis::widget& owner_widget, //
	size_t food_index = std::numeric_limits<size_t>::max()
);

} // namespace calslog

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

#include <ruis/widget/widget.hpp>

namespace calslog {

// Opens the food edit dialog for the food at the given index in the model's foods list.
// The dialog is pre-filled with the food's current name, kcal/100g and mass per serving.
void show_food_edit_dialog(ruis::widget& owner_widget, size_t food_index);

} // namespace calslog

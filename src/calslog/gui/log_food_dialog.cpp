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

#include "log_food_dialog.hpp"

#include <functional>
#include <string_view>

#include <ruis/widget/button/impl/rectangle_push_button.hpp>
#include <ruis/widget/group/overlay.hpp>
#include <ruis/widget/group/touch/dialog.hpp>
#include <ruis/widget/input/labeled_text_field.hpp>
#include <ruis/widget/label/gap.hpp>
#include <ruis/widget/label/text.hpp>
#include <ruis/widget/widget.hpp>

#include "style.hpp"

using namespace std::string_literals;
using namespace std::string_view_literals;

namespace {

// Returns true if the given string contains a decimal point.
bool has_decimal_point(std::u32string_view s)
{
	for (auto ch : s) {
		if (ch == U'.') {
			return true;
		}
	}
	return false;
}

} // namespace

namespace calslog {

void show_log_food_dialog(ruis::widget& parent_widget)
{
	auto& c = parent_widget.context;

	auto& olay = parent_widget.get_ancestor<ruis::overlay>();

	// Create the Add button separately so we can set its click handler
	// clang-format off
	auto add_button = m::rectangle_push_button(c,
		{
			.layout_params{
				.dims = {ruis::dim::fill, ruis::dim::min},
				.weight = 1,
				.align = {ruis::align::center, ruis::align::back}
			},
			.params{
				.rectangle_button{
					.specific{
						// TODO: add special button?
						.unpressed_color = c.get().style().get_color_special()
					}
				}
			}
		},
		{
			m::text(c, {}, c.get().localization.get().get("log_food_dialog:add_button"sv))
		}
	);
	// clang-format on

	// Helper to create a labeled text field with a localized label and hint.
	// An optional input filter may be provided to restrict what can be typed.
	auto make_field = [&c](
						  std::string_view label_loc_id, //
						  std::string_view hint_loc_id,
						  std::function<bool(std::u32string_view, size_t, size_t, std::u32string_view)> filter = {}
					  ) {
		return m::labeled_text_field(
			c,
			{
				.layout_params{.dims = {ruis::dim::fill, ruis::dim::min}},
				.params{
							   .label{.string = c.get().localization.get().get(label_loc_id)},
							   .text_input{.specific{.hint = c.get().localization.get().get(hint_loc_id), .filter = filter}}
				}
        },
			ruis::string()
		);
	};

	// Helper to create a gap with the standard vertical spacing
	auto make_gap = [&c]() {
		return m::gap(c, {.layout_params{.dims = {ruis::dim::fill, c.get().style().get_len_gap()}}});
	};

	// Input filter that restricts a field to a numeric value:
	// only digits and at most a single decimal point are allowed.
	// The filter is stateless, so it is stored once and copied into each field that needs it.
	auto numeric_filter =
		[](std::u32string_view original, size_t replace_start, size_t replace_end, std::u32string_view to_insert) {
			bool insert_has_dot = false;
			for (auto ch : to_insert) {
				if (ch == U'.') {
					if (insert_has_dot) {
						return false;
					}
					insert_has_dot = true;
				} else if (ch < U'0' || ch > U'9') {
					return false;
				}
			}
			const bool remaining_has_dot = has_decimal_point(std::u32string_view(original.data(), replace_start)) || //
				has_decimal_point(std::u32string_view(original.data() + replace_end, original.size() - replace_end));
			return !(remaining_has_dot && insert_has_dot);
		};

	// Create the input fields as separate variables so that the validator below
	// can observe their text and control the enabled state of the Add button.
	auto food_name_field = make_field(
		"log_food_dialog:food_name"sv, //
		"log_food_dialog:food_name_hint"sv
	);
	auto calories_field = make_field(
		"log_food_dialog:calories_per_100g"sv, //
		"log_food_dialog:calories_per_100g_hint"sv,
		numeric_filter
	);
	auto mass_field = make_field(
		"log_food_dialog:food_mass"sv, //
		"log_food_dialog:food_mass_hint"sv,
		numeric_filter
	);

	// Validator: the Add button is enabled only when all three fields are non-empty.
	// The fields and the button are owned by the dialog and outlive this function, so
	// it is safe to capture them by reference in the change handlers.
	auto& fn_input = food_name_field.get().get_text_input();
	auto& cal_input = calories_field.get().get_text_input();
	auto& mass_input = mass_field.get().get_text_input();

	// Recompute and apply the enabled state of the Add button.
	auto update_add_button_enabled = [&add_btn = add_button.get(), //
									  &fn_input,
									  &cal_input,
									  &mass_input]() //
	{
		const bool all_filled = //
			!fn_input.get_string().empty() && //
			!cal_input.get_string().empty() && //
			!mass_input.get_string().empty();
		add_btn.set_enabled(all_filled);
	};

	// Watch the text of each field and recompute the button enabled state on change.
	auto watch_field = [update_add_button_enabled](auto& input) {
		input.text_change_handler = [update_add_button_enabled](ruis::text_widget&) {
			update_add_button_enabled();
		};
	};
	watch_field(fn_input);
	watch_field(cal_input);
	watch_field(mass_input);

	// Set the initial enabled state (all fields are empty -> the button is disabled).
	update_add_button_enabled();

	// Create the dialog with its content
	// clang-format off
	auto dialog = ruis::touch::make::dialog(c,
		{
			.layout_params{
				.dims = {ruis::dim::fill, ruis::dim::fill}
			}
		},
		{
			m::text(c,
				{
					.params{
						.font{
							.size = c.get().style().get_font_size_title()
						}
					}
				},
				c.get().localization.get().get("log_food_dialog:title"sv)
			),
			make_gap(),
			std::move(food_name_field),
			make_gap(),
			std::move(calories_field),
			make_gap(),
			std::move(mass_field),
			make_gap(),
			std::move(add_button)
		}
	);
	// clang-format on

	// Show the dialog
	c.get().post_to_ui_thread([olay = utki::make_shared_from(olay), dialog]() {
		olay.get().push_back(dialog);
	});
}

} // namespace calslog

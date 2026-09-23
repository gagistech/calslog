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
#include <utki/string.hpp>
#include <utki/unicode.hpp>

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

// Converts a numeric UTF-32 string to a float.
// Empty or unparsable input yields 0.
float to_float(const std::u32string& s)
{
	auto s8 = utki::to_utf8(s);
	float value = 0;
	utki::from_chars(s8.data(), s8.data() + s8.size(), value);
	return value;
}

} // namespace

namespace calslog {

void show_log_food_dialog(ruis::widget& parent_widget)
{
	auto& c = parent_widget.context;

	auto& olay = parent_widget.get_ancestor<ruis::overlay>();

	// Helper to create a push button with a localized caption and the given color.
	auto make_button = [&c](std::string_view text_loc_id, ruis::styled<ruis::color> color) {
		// clang-format off
		return m::rectangle_push_button(c,
			{
				.layout_params{
					.dims = {ruis::dim::fill, ruis::dim::min},
					.weight = 1
				},
				.params{
					.rectangle_button{
						.specific{
							.unpressed_color = color
						}
					}
				}
			},
			{
				m::text(c, {}, c.get().localization.get().get(text_loc_id))
			}
		);
		// clang-format on
	};

	// Create the Add button separately so we can reference it in the validator below.
	auto add_button = make_button("log_food_dialog:add_button"sv, c.get().style().get_color_special());
	auto cancel_button = make_button("log_food_dialog:cancel_button"sv, c.get().style().get_color_primary());

	// The Cancel button closes the dialog.
	cancel_button.get().click_handler = [](ruis::push_button& b) {
		b.get_ancestor<ruis::touch::dialog>().close();
	};

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
	auto make_vert_gap = [&c]() {
		return m::gap(c, {.layout_params{.dims = {ruis::dim::fill, c.get().style().get_len_gap()}}});
	};

	// Helper to create a horizontal gap (used to separate the dialog buttons)
	auto make_hori_gap = [&c]() {
		return m::gap(c, {.layout_params{.dims = {c.get().style().get_len_gap(), ruis::dim::fill}}});
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

	// The label showing the total kcal for the entered kcal/100g and mass.
	auto total_label = m::text(
		c,
		{.layout_params{.dims = {ruis::dim::fill, ruis::dim::min}},
		 .params{.font{.size = c.get().style().get_font_size_primary()}}},
		std::u32string{}
	);

	// Recompute and apply the total label text from the current field values.
	// Total kcal = (entered kcal/100g) * (entered mass in grams) / 100.
	auto update_total_label = [c, //
								   & total_lbl = total_label.get(),
							   &cal_input,
							   &mass_input]() //
	{
		const float kcal_per_100g = to_float(cal_input.get_string());
		const float mass_g = to_float(mass_input.get_string());
		const float total_kcal = kcal_per_100g * mass_g / 100.f;
		const auto total_str = utki::to_utf32(utki::cat(total_kcal));
		total_lbl.set_text(c.get().localization.get().get("log_food_dialog:total"sv).format({total_str}).string());
	};

	// Recompute both the Add button enabled state and the total label on any field change.
	auto update_all = [update_add_button_enabled, update_total_label]() //
	{
		update_add_button_enabled();
		update_total_label();
	};

	// Watch the text of each field and recompute the button enabled state and total on change.
	auto watch_field = [update_all](auto& input) {
		input.text_change_handler = [update_all](ruis::text_widget&) {
			update_all();
		};
	};
	watch_field(fn_input);
	watch_field(cal_input);
	watch_field(mass_input);

	// Set the initial state (all fields are empty -> the button is disabled and the total is 0).
	update_all();

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
			make_vert_gap(),
			std::move(food_name_field),
			make_vert_gap(),
			std::move(calories_field),
			make_vert_gap(),
			std::move(mass_field),
			make_vert_gap(),
			std::move(total_label),
			make_vert_gap(),
			m::row(c,
				{
					.layout_params{
						.dims = {ruis::dim::fill, ruis::dim::min},
						.weight = 1,
						.align = {ruis::align::front, ruis::align::back}
					}
				},
				{
					std::move(cancel_button),
					make_hori_gap(),
					std::move(add_button)
				}
			)
		}
	);
	// clang-format on

	// Show the dialog
	c.get().post_to_ui_thread([olay = utki::make_shared_from(olay), dialog]() {
		olay.get().push_back(dialog);
	});
}

} // namespace calslog

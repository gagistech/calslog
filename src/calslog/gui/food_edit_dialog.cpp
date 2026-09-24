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

#include "food_edit_dialog.hpp"

#include <cstdint>
#include <functional>
#include <string>
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

#include "../application.hpp"
#include "../model/model.hpp"

#include "style.hpp"

using namespace std::string_literals;
using namespace std::string_view_literals;

namespace {
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

void show_food_edit_dialog(ruis::widget& owner_widget, size_t food_index)
{
	auto& c = owner_widget.context;

	auto& olay = owner_widget.get_ancestor<ruis::overlay>();

	// Current values of the food being edited, used to pre-fill the fields.
	const auto& food = application::inst().model.foods.at(food_index);

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

	// Create the Save and Cancel buttons.
	auto save_button = make_button("food_edit_dialog:save_button"sv, c.get().style().get_color_special());
	auto cancel_button = make_button("food_edit_dialog:cancel_button"sv, c.get().style().get_color_primary());

	// The Cancel button closes the dialog.
	cancel_button.get().click_handler = [](ruis::push_button& b) {
		b.get_ancestor<ruis::touch::dialog>().close();
	};

	// Helper to create a labeled text field with a localized label and hint.
	// An optional input filter may be provided to restrict what can be typed.
	auto make_field = [&c](
						  std::string_view label_loc_id, //
						  std::string_view hint_loc_id,
						  std::function<bool(std::u32string_view, size_t, size_t, std::u32string_view)> filter = {}, //
						  ruis::string initial = {} //
					  ) {
		return m::labeled_text_field(
			c,
			{
				.layout_params{.dims = {ruis::dim::fill, ruis::dim::min}},
				.params{
							   .label{.string = c.get().localization.get().get(label_loc_id)},
							   .text_input{.specific{.hint = c.get().localization.get().get(hint_loc_id), .filter = filter}}}
			},
			initial
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

	// Factory that creates an input filter restricting a field to a positive integer
	// of at most `max_digits` digits. The filter is stateless, so the produced
	// std::function can be copied into each field that needs it.
	auto make_numeric_filter = [](size_t max_digits) {
		return [max_digits](
				   std::u32string_view original,
				   size_t replace_start,
				   size_t replace_end,
				   std::u32string_view to_insert
			   ) -> bool {
			for (auto ch : to_insert) {
				if (ch < U'0' || ch > U'9') {
					return false;
				}
			}
			// Limit the resulting string to at most max_digits digits. The result is
			// the original string with the [replace_start, replace_end) range replaced
			// by to_insert.
			const size_t result_length = original.size() - (replace_end - replace_start) + to_insert.size();
			return result_length <= max_digits;
		};
	};

	// Create the input fields, pre-filled with the food's current values.
	auto food_name_field = make_field(
		"food_edit_dialog:food_name"sv, //
		"food_edit_dialog:food_name_hint"sv,
		{}, // no input filter
		ruis::string(food.name)
	);
	auto calories_field = make_field(
		"food_edit_dialog:calories_per_100g"sv, //
		"food_edit_dialog:calories_per_100g_hint"sv,
		make_numeric_filter(4),
		ruis::string(utki::to_utf32(utki::to_string(food.kcal)))
	);
	auto mass_field = make_field(
		"food_edit_dialog:mass_per_serving"sv, //
		"food_edit_dialog:mass_per_serving_hint"sv,
		make_numeric_filter(4),
		ruis::string(utki::to_utf32(utki::to_string(food.mass)))
	);

	// The Save button is enabled only when all three fields are non-empty.
	// The fields and the button are owned by the dialog and outlive this function, so
	// it is safe to capture them by reference in the change handlers.
	auto& name_input = food_name_field.get().get_text_input();
	auto& cal_input = calories_field.get().get_text_input();
	auto& mass_input = mass_field.get().get_text_input();

	// Recompute and apply the enabled state of the Save button.
	auto update_save_button_enabled = [&save_btn = save_button.get(),
									   &name_input,
									   &cal_input,
									   &mass_input]() //
	{
		const bool all_filled = //
			!name_input.get_string().empty() && //
			!cal_input.get_string().empty() && //
			!mass_input.get_string().empty();
		save_btn.set_enabled(all_filled);
	};

	// Watch the text of each field and recompute the Save button enabled state on change.
	auto watch_field = [update_save_button_enabled](auto& input) {
		input.text_change_handler = [update_save_button_enabled](ruis::text_widget&) {
			update_save_button_enabled();
		};
	};
	watch_field(name_input);
	watch_field(cal_input);
	watch_field(mass_input);

	// Set the initial state.
	update_save_button_enabled();

	// The Save button updates the food in the model, emits the model change signal
	// and closes the dialog. The fields are owned by the dialog and outlive this
	// function, so it is safe to capture them by reference.
	save_button.get().click_handler = [&name_input, &cal_input, &mass_input, food_index](ruis::push_button& b) {
		auto& food = application::inst().model.foods.at(food_index);
		food.name = name_input.get_string();
		food.kcal = uint32_t(to_float(cal_input.get_string()));
		food.mass = uint32_t(to_float(mass_input.get_string()));
		application::inst().model.model_changed_signal.emit();
		b.get_ancestor<ruis::touch::dialog>().close();
	};

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
				c.get().localization.get().get("food_edit_dialog:title"sv)
			),
			make_vert_gap(),
			std::move(food_name_field),
			make_vert_gap(),
			std::move(calories_field),
			make_vert_gap(),
			std::move(mass_field),
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
					std::move(save_button)
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

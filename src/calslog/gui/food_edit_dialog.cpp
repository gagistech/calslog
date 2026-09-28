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

#include <cmath>
#include <cstdint>
#include <functional>
#include <limits>
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

void show_food_edit_dialog(
	ruis::widget& owner_widget, //
	size_t food_index
)
{
	auto& c = owner_widget.context;

	auto& olay = owner_widget.get_ancestor<ruis::overlay>();

	// In edit mode the dialog opens prefilled with the given food and its primary
	// button reads "Save" and updates that food in place; otherwise (add mode) it
	// opens empty, its primary button reads "Add" and submitting appends a new food
	// to the model's foods list.
	const bool is_editing = food_index != std::numeric_limits<size_t>::max();
	model::food food{};
	if (is_editing) {
		food = application::inst().model.foods.at(food_index);
	}

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
	// The primary button's caption is "Save" when editing an existing food,
	// otherwise "Add".
	auto save_button = make_button(
		is_editing ? "food_edit_dialog:save_button"sv : "food_edit_dialog:add_button"sv,
		c.get().style().get_color_special()
	);
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
			// clang-format off
			{
				.layout_params{
					.dims = {ruis::dim::fill, ruis::dim::min}
				},
				.params{
					.label{
						.string = c.get().localization.get().get(label_loc_id)
					},
					.text_input{
						.specific{
							.hint = c.get().localization.get().get(hint_loc_id),
							.filter = filter
						}
					}
				}
			},
			// clang-format on
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

	// Create the input fields, pre-filled with the food's current values when
	// editing, left empty when adding.
	auto food_name_field = make_field(
		"food_edit_dialog:food_name"sv, //
		"food_edit_dialog:food_name_hint"sv,
		{}, // no input filter
		is_editing ? ruis::string(food.name) : ruis::string{}
	);
	auto calories_field = make_field(
		"food_edit_dialog:calories_per_100g"sv, //
		"food_edit_dialog:calories_per_100g_hint"sv,
		make_numeric_filter(4),
		is_editing ? ruis::string(utki::to_utf32(utki::to_string(food.kcal))) : ruis::string{}
	);
	auto mass_field = make_field(
		"mass_per_serving"sv, //
		"mass_per_serving_hint"sv,
		make_numeric_filter(4),
		is_editing ? ruis::string(utki::to_utf32(utki::to_string(food.mass))) : ruis::string{}
	);

	// The Save button is enabled only when all three fields are non-empty.
	// The fields and the button are owned by the dialog and outlive this function, so
	// it is safe to capture them by reference in the change handlers.
	auto& name_input = food_name_field.get().get_text_input();
	auto& cal_input = calories_field.get().get_text_input();
	auto& mass_input = mass_field.get().get_text_input();

	// The label showing the total calories per portion, computed from the entered
	// kcal/100g and mass per serving, in the default text color. It is updated live
	// as the user types. The dialog owns it and outlives this function, so it is
	// safe to capture it by reference in the handlers below.
	auto portion_kcal_label = m::text(
		c,
		// clang-format off
		{
			.layout_params{
				.dims = {ruis::dim::fill, ruis::dim::min}
			},
			.params{
				.color = c.get().style().get_color_text()
			}
		},
		// clang-format on
		std::u32string{}
	);

	// Recompute and apply the enabled state of the Save button.
	auto update_save_button_enabled = [&save_btn = save_button.get(),
									   &name_input,
									   &cal_input,
									   &mass_input]() //
	{
		const bool all_filled = //
			!name_input.get_string().get().empty() && //
			!cal_input.get_string().get().empty() && //
			!mass_input.get_string().get().empty();
		save_btn.set_enabled(all_filled);
	};

	// Recompute the total-calories-per-portion label from the current kcal/100g and
	// mass per serving field values. Total per portion = (kcal/100g) * (mass per
	// serving in grams) / 100, rounded to the nearest integer. Shown as "?" while
	// either of the two fields is empty.
	auto update_portion_kcal = [&lbl = portion_kcal_label.get(), &cal_input, &mass_input, c]() //
	{
		const auto& cal_str = cal_input.get_string().get();
		const auto& mass_str = mass_input.get_string().get();

		std::u32string value_str;
		if (!cal_str.empty() && !mass_str.empty()) {
			const float kcal_per_100g = to_float(cal_str);
			const float mass_g = to_float(mass_str);
			const uint32_t total = std::round(kcal_per_100g * mass_g / 100.f);
			value_str = utki::to_utf32(std::to_string(total));
		} else {
			value_str = std::u32string(U"?");
		}
		lbl.set_string(c.get().localization.get().get("foods_page:portion_kcal"sv).format({value_str}).string());
	};

	// Recompute both the Save button enabled state and the portion-kcal label.
	auto update_all = [update_save_button_enabled, update_portion_kcal]() //
	{
		update_save_button_enabled();
		update_portion_kcal();
	};

	// Watch the text of each field and recompute the Save button enabled state and
	// the portion-kcal label on change.
	auto watch_field = [update_all](auto& input) {
		input.text_change_handler = [update_all](auto&) {
			update_all();
		};
	};
	watch_field(name_input);
	watch_field(cal_input);
	watch_field(mass_input);

	// Set the initial state.
	update_all();

	// The Save button updates the food in the model, emits the model change signal
	// and closes the dialog. The fields are owned by the dialog and outlive this
	// function, so it is safe to capture them by reference.
	save_button.get().click_handler =
		[&name_input, &cal_input, &mass_input, is_editing, food_index](ruis::push_button& b) {
			auto& app = application::inst();
			if (is_editing) {
				auto& target = app.model.foods.at(food_index);
				target.name = name_input.get_string().get();
				target.kcal = uint32_t(to_float(cal_input.get_string().get()));
				target.mass = uint32_t(to_float(mass_input.get_string().get()));
			} else {
				app.model.foods.push_back(model::food{
					.name = name_input.get_string().get(),
					.kcal = uint32_t(to_float(cal_input.get_string().get())),
					.mass = uint32_t(to_float(mass_input.get_string().get()))
				});
			}
			app.model.model_changed_signal.emit();
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
				c.get().localization.get().get(
					is_editing ? "food_edit_dialog:title"sv : "food_edit_dialog:add_title"sv)
			),
			make_vert_gap(),
			std::move(food_name_field),
			make_vert_gap(),
			std::move(calories_field),
			make_vert_gap(),
			std::move(mass_field),
			make_vert_gap(),
			std::move(portion_kcal_label),
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

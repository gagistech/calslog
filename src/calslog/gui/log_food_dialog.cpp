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

#include <cmath>
#include <cstdint>
#include <functional>
#include <limits>
#include <string>
#include <string_view>

#include <ruis/widget/button/impl/rectangle_push_button.hpp>
#include <ruis/widget/group/overlay.hpp>
#include <ruis/widget/group/touch/dialog.hpp>
#include <ruis/widget/input/impl/rectangle_text_field.hpp>
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

// Returns the given string, or U"?" if it is empty.
std::u32string or_unknown(std::u32string_view s)
{
	return s.empty() ? std::u32string(U"?") : std::u32string(s);
}

} // namespace

namespace calslog {

void show_log_food_dialog(ruis::widget& owner_widget, size_t edit_entry_index)
{
	auto& c = owner_widget.context;

	auto& olay = owner_widget.get_ancestor<ruis::overlay>();

	// In edit mode the dialog opens prefilled with the given entry and its primary
	// button reads "Save" and updates that entry in place; otherwise (add mode) it
	// opens empty and appends a new entry to today.
	const bool is_editing = edit_entry_index != std::numeric_limits<size_t>::max();
	model::entry editing_entry{};
	if (is_editing) {
		editing_entry = application::inst().model.today.entries.at(edit_entry_index);
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

	// Create the primary button separately so we can reference it in the validator
	// below. Its caption is "Save" when editing an existing entry, otherwise "Add".
	auto add_button = make_button(
		is_editing ? "log_food_dialog:save_button"sv : "log_food_dialog:add_button"sv,
		c.get().style().get_color_special()
	);
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
						  std::function<bool(std::u32string_view, size_t, size_t, std::u32string_view)> filter = {}, //
						  ruis::string initial = {}, //
						  int weight = -1 //
					  ) {
		return m::labeled_text_field(
			c,
			{
				.layout_params{.dims = {ruis::dim::fill, ruis::dim::min},.weight = weight                                                               },
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
	// Factory that creates an input filter restricting a field to a decimal number
	// with at most 2 digits before the dot and at most 2 digits after it. The dot is
	// the only allowed non-digit character and may appear at most once.
	auto make_decimal_filter = []() {
		return [](std::u32string_view original, size_t replace_start, size_t replace_end, std::u32string_view to_insert
			   ) -> bool {
			// Only digits and at most one dot may be inserted.
			size_t dots_in_insert = 0;
			for (auto ch : to_insert) {
				if (ch == U'.') {
					++dots_in_insert;
				} else if (ch < U'0' || ch > U'9') {
					return false;
				}
			}
			if (dots_in_insert > 1) {
				return false;
			}
			// Build the resulting string: the original with [replace_start, replace_end)
			// replaced by to_insert, then validate the digit limits.
			std::u32string result;
			result.append(original.substr(0, replace_start));
			result.append(to_insert);
			result.append(original.substr(replace_end));
			size_t dots = 0;
			size_t digits_before = 0;
			size_t digits_after = 0;
			for (auto ch : result) {
				if (ch == U'.') {
					++dots;
				} else if (ch >= U'0' && ch <= U'9') {
					if (dots == 0) {
						++digits_before;
					} else {
						++digits_after;
					}
				}
			}
			if (dots > 1) {
				return false;
			}
			if (dots == 0) {
				return digits_before <= 2;
			}
			return digits_before <= 2 && digits_after <= 2;
		};
	};
	// Create the input fields as separate variables so that the validator below
	// can observe their text and control the enabled state of the Add button.
	auto food_name_field = make_field(
		"log_food_dialog:food_name"sv, //
		"log_food_dialog:food_name_hint"sv,
		{}, //
		is_editing ? ruis::string(editing_entry.name) : ruis::string{}
	);
	auto calories_field = make_field(
		"log_food_dialog:calories_per_100g"sv, //
		"log_food_dialog:calories_per_100g_hint"sv,
		make_numeric_filter(4), //
		is_editing ? ruis::string(utki::to_utf32(utki::to_string(editing_entry.kcal))) : ruis::string{}
	);
	auto mass_field = make_field(
		"log_food_dialog:food_mass"sv, //
		"log_food_dialog:food_mass_hint"sv,
		make_numeric_filter(4), //
		is_editing ? ruis::string(utki::to_utf32(utki::to_string(editing_entry.mass))) : ruis::string{}
	);
	// The number of servings field is built manually (instead of make_field) so
	// that the row of preset buttons can be placed on a separate row between the
	// field name and the field itself.
	auto num_servings_label = m::text(
		c,
		// clang-format off
		{
			.layout_params{
				.dims = {ruis::dim::fill, ruis::dim::min},
				.align = {ruis::align::front, ruis::align::center}
			},
			.params{
				.color = c.get().style().get_color_text()
			}
		},
		// clang-format on
		c.get().localization.get().get("log_food_dialog:num_servings"sv)
	);
	auto num_servings_input = m::rectangle_text_field(
		c,
		// clang-format off
		{
			.layout_params{.dims = {ruis::dim::fill, ruis::dim::min}},
			.params{
				.text_input{
					.specific{
						.hint = c.get().localization.get().get("log_food_dialog:num_servings_hint"sv),
						.filter = make_decimal_filter()
					}
				}
			}
		},
		// clang-format on
		is_editing ? ruis::string(utki::to_utf32(utki::to_string(editing_entry.pcs))) : ruis::string(U"1")
	);
	auto& pcs_input = num_servings_input.get().get_text_input();

	// Helper to create a small push button showing a number; pressing it sets the
	// number of pieces field to that number.
	auto make_num_button = [&c, &pcs_input](std::u32string number) {
		// clang-format off
		// All preset buttons have weight 1, so they are of equal width and
		// together occupy the whole horizontal space of the dialog.
		auto button = m::rectangle_push_button(c,
			{
				.layout_params{
					.dims = {ruis::dim::fill, ruis::dim::min},
					.weight = 1
				}
			},
			{
				m::text(c, {}, number)
			}
		);
		// clang-format on
		button.get().click_handler = [number, &pcs_input](ruis::push_button&) {
			pcs_input.set_text(number);
		};
		return button;
	};

	// Assemble the number of servings field: the field name, then the row of preset
	// buttons, then a small gap, then the field itself.
	auto num_servings_field = m::column(
		c,
		// clang-format off
		{
			.layout_params{
				.dims = {ruis::dim::fill, ruis::dim::min}
			}
		},
		{
			std::move(num_servings_label),
			m::row(c,
				{
					.layout_params{
						.dims = {ruis::dim::fill, ruis::dim::min}
					}
				},
				{
					make_num_button(std::u32string(U"0.25")), //
					make_hori_gap(),
					make_num_button(std::u32string(U"0.5")), //
					make_hori_gap(),
					make_num_button(std::u32string(U"2")), //
					make_hori_gap(),
					make_num_button(std::u32string(U"3")), //
					make_hori_gap(),
					make_num_button(std::u32string(U"4"))
				}
			),
			m::gap(c, {.layout_params{.dims = {0, c.get().style().get_len_gap_small()}}}),
			std::move(num_servings_input)
		} // clang-format on
	);

	// Validator: the Add button is enabled only when all four fields are non-empty.
	// The fields and the button are owned by the dialog and outlive this function, so
	// it is safe to capture them by reference in the change handlers.
	auto& fn_input = food_name_field.get().get_text_input();
	auto& cal_input = calories_field.get().get_text_input();
	auto& mass_input = mass_field.get().get_text_input();

	// Recompute and apply the enabled state of the Add button.
	auto update_add_button_enabled = [&add_btn = add_button.get(), //
									  &fn_input,
									  &cal_input,
									  &mass_input,
									  &pcs_input]() //
	{
		const bool all_filled = //
			!fn_input.get_string().empty() && //
			!cal_input.get_string().empty() && //
			!mass_input.get_string().empty() && //
			!pcs_input.get_string().empty();
		add_btn.set_enabled(all_filled);
	};

	// The label showing the "XX pcs x YY g x ZZ kcal/100g" detail, in secondary
	// color and font size, placed right above the total kcal label.
	// The detail and total labels are right-aligned, so that they sit at the
	// right bottom of the dialog.
	auto detail_label = m::text(
		c,
		// clang-format off
		{
			.layout_params{
				.dims = {ruis::dim::min, ruis::dim::min},
				.align = {ruis::align::back, ruis::align::center}
			},
			.params{
				.color = c.get().style().get_color_text_secondary(),
				.font{
					.size = c.get().style().get_font_size_secondary()
				}
			}
    	},
		// clang-format on
		std::u32string{}
	);

	// The label showing the total kcal for the entered kcal/100g and mass.
	auto total_label = m::text(
		c,
		// clang-format off
		{
			.layout_params{
				.dims = {ruis::dim::min, ruis::dim::min},
				.align = {ruis::align::back, ruis::align::center}
			},
			.params{
				.color = c.get().style().get_color_text_special()
			}
		},
		// clang-format on
		std::u32string{}
	);

	// Recompute and apply the detail and total labels from the current field values.
	// Total kcal = (entered kcal/100g) * (entered mass in grams per piece) *
	// (entered number of pieces) / 100. The total is rounded to the nearest
	// integer since fractions of kcal are not representative.
	auto update_labels = [c, //
							  & detail_lbl = detail_label.get(),
						  &total_lbl = total_label.get(),
						  &cal_input,
						  &mass_input,
						  &pcs_input]() //
	{
		const auto& pcs_str = pcs_input.get_string();
		const auto& mass_str = mass_input.get_string();
		const auto& cal_str = cal_input.get_string();

		const bool all_filled = !pcs_str.empty() && !mass_str.empty() && !cal_str.empty();

		detail_lbl.set_text(c.get()
								.localization.get()
								.get("log_food_dialog:entry_detail"sv)
								.format({or_unknown(pcs_str), or_unknown(mass_str), or_unknown(cal_str)})
								.string());

		std::u32string total_str;
		if (all_filled) {
			const float kcal_per_100g = to_float(cal_str);
			const float mass_g = to_float(mass_str);
			const float pcs = to_float(pcs_str);
			const uint32_t total_kcal = std::round(kcal_per_100g * mass_g * pcs / 100.f);
			total_str = utki::to_utf32(std::to_string(total_kcal));
		} else {
			total_str = std::u32string(U"?");
		}
		total_lbl.set_text(c.get().localization.get().get("total_kcal"sv).format({total_str}).string());
	};

	// Recompute both the Add button enabled state and the labels on any field change.
	auto update_all = [update_add_button_enabled, update_labels]() //
	{
		update_add_button_enabled();
		update_labels();
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
	watch_field(pcs_input);

	// Set the initial state (all fields are empty -> the button is disabled and the total is 0).
	update_all();

	// The primary button commits the form: in add mode it appends a new entry to the
	// model's today entries, in edit mode it updates the entry being edited. It then
	// emits the model change signal and closes the dialog. The fields are owned by the
	// dialog and outlive this function, so it is safe to capture them by reference.
	add_button.get().click_handler =
		[&fn_input, &cal_input, &mass_input, &pcs_input, is_editing, edit_entry_index](ruis::push_button& b) {
			auto& app = application::inst();
			if (is_editing) {
				auto& e = app.model.today.entries.at(edit_entry_index);
				e.name = fn_input.get_string();
				e.pcs = to_float(pcs_input.get_string());
				e.mass = uint32_t(to_float(mass_input.get_string()));
				e.kcal = uint32_t(to_float(cal_input.get_string()));
			} else {
				app.model.today.entries.push_back(model::entry{
					.name = fn_input.get_string(),
					.pcs = to_float(pcs_input.get_string()),
					.mass = uint32_t(to_float(mass_input.get_string())),
					.kcal = uint32_t(to_float(cal_input.get_string()))
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
				c.get().localization.get().get("log_food_dialog:title"sv)
			),
			make_vert_gap(),
			std::move(food_name_field),
			make_vert_gap(),
			std::move(calories_field),
			make_vert_gap(),
			std::move(mass_field),
			make_vert_gap(),
			std::move(num_servings_field),
			make_vert_gap(),
			std::move(detail_label),
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

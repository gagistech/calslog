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

#include "log_weight_dialog.hpp"

#include <cmath>
#include <cstdint>
#include <cstdio>
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
	utki::string_parser p(s8);
	return p.read_number<float>();
}

// Formats a weight given in grams as a kilograms string with at most one digit
// after the decimal point (e.g. 72500 -> "72.5", 72000 -> "72"). The value is
// rounded to the nearest 0.1 kg, matching the format used to display the weight
// on the today page.
std::u32string grams_to_kg_string(uint32_t grams)
{
	// Round to the nearest 0.1 kg (100 g) using integer arithmetic.
	const uint32_t tenths = (grams + 50) / 100;
	const uint32_t kg = tenths / 10;
	const uint32_t frac = tenths % 10;

	if (frac == 0) {
		return utki::to_utf32(std::to_string(kg));
	}

	char buf[16];
	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
	std::snprintf(buf, sizeof buf, "%u.%u", static_cast<unsigned>(kg), static_cast<unsigned>(frac));
	return utki::to_utf32(buf);
}
} // namespace

namespace calslog {

void show_log_weight_dialog(ruis::widget& owner_widget)
{
	auto& c = owner_widget.context;

	auto& olay = owner_widget.get_ancestor<ruis::overlay>();

	// The weight is stored in the model in grams; 0 means it was not logged yet,
	// in which case the dialog opens with an empty field.
	const uint32_t current_weight = application::inst().model.today.weight;

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
	auto save_button = make_button("log_weight_dialog:save_button"sv, c.get().style().get_color_special());
	auto cancel_button = make_button("log_weight_dialog:cancel_button"sv, c.get().style().get_color_primary());

	// The Cancel button closes the dialog.
	cancel_button.get().click_handler = [](ruis::push_button& b) {
		b.get_ancestor<ruis::touch::dialog>().close();
	};

	// Helper to create a gap with the standard vertical spacing
	auto make_vert_gap = [&c]() {
		return m::gap(c, {.layout_params{.dims = {ruis::dim::fill, c.get().style().get_len_gap()}}});
	};

	// Helper to create a horizontal gap (used to separate the dialog buttons)
	auto make_hori_gap = [&c]() {
		return m::gap(c, {.layout_params{.dims = {c.get().style().get_len_gap(), ruis::dim::fill}}});
	};

	// Factory that creates an input filter restricting a field to a decimal number
	// with at most 3 digits before the dot and at most 1 digit after it. The dot
	// is the only allowed non-digit character and may appear at most once.
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
				return digits_before <= 3;
			}
			return digits_before <= 3 && digits_after <= 1;
		};
	};

	// Create the weight field, prefilled with the current weight (if any) formatted
	// in kilograms.
	auto weight_field = m::labeled_text_field(
		c,
		// clang-format off
		{
			.layout_params{
				.dims = {ruis::dim::fill, ruis::dim::min}
			},
			.params{
				.label{
					.string = c.get().localization.get().get("log_weight_dialog:weight"sv)
				},
				.text_input{
					.specific{
						.hint = c.get().localization.get().get("log_weight_dialog:weight_hint"sv),
						.filter = make_decimal_filter()
					}
				}
			}
		},
		// clang-format on
		current_weight == 0 ? ruis::string{} : ruis::string(grams_to_kg_string(current_weight))
	);

	// The Save button is enabled only when the weight field is non-empty.
	// The field and the button are owned by the dialog and outlive this function, so
	// it is safe to capture them by reference in the change handler.
	auto& weight_input = weight_field.get().get_text_input();

	// Recompute and apply the enabled state of the Save button.
	auto update_save_button_enabled = [&save_btn = save_button.get(), &weight_input]() {
		save_btn.set_enabled(!weight_input.get_string().get().empty());
	};

	// Watch the weight field and recompute the Save button enabled state on change.
	weight_input.text_change_handler = [update_save_button_enabled](auto&) {
		update_save_button_enabled();
	};

	// Set the initial state.
	update_save_button_enabled();

	// The Save button updates the model's today weight (in grams) from the entered
	// kilograms value, emits the model change signal and closes the dialog.
	// The field is owned by the dialog and outlives this function, so it is safe
	// to capture it by reference.
	save_button.get().click_handler = [&weight_input](ruis::push_button& b) {
		auto& app = application::inst();
		const float kg = to_float(weight_input.get_string().get());
		app.model.today.weight = static_cast<uint32_t>(std::lround(kg * 1000.f));
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
				c.get().localization.get().get("log_weight_dialog:title"sv)
			),
			make_vert_gap(),
			std::move(weight_field),
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

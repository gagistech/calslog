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

#include "settings_dialog.hpp"

#include <cstdio>
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
#include "../settings.hpp"
#include "style.hpp"

using namespace std::string_view_literals;

namespace {
// Formats the time of day, given as minutes after midnight, as "H:MM".
std::u32string format_day_flip_time(uint32_t minutes)
{
	char buf[32];
	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
	std::snprintf(buf, sizeof buf, "%u:%02u", minutes / 60, minutes % 60);
	return utki::to_utf32(buf);
}

// Parses the time of day, given as an "H:MM" or "HH:MM" string,
// and returns it as minutes after midnight.
// Throws std::invalid_argument if the input is not a valid time of day.
uint32_t parse_day_flip_time(std::string_view str)
{
	try {
		utki::string_parser p(str);
		auto hours = p.read_number<uint32_t>();
		p.skip_char(':');
		auto minutes = p.read_number<uint32_t>();
		p.skip_whitespaces();
		if (!p.get_view().empty() || hours > 23 || minutes > 59) {
			throw std::invalid_argument("invalid time of day");
		}
		return hours * 60 + minutes;
	} catch (...) {
		// std::string_parser also throws std::invalid_argument on malformed input
		throw std::invalid_argument("invalid time of day");
	}
}
} // namespace

namespace calslog {

void show_settings_dialog(ruis::widget& owner_widget)
{
	auto& c = owner_widget.context;

	auto& olay = owner_widget.get_ancestor<ruis::overlay>();

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
	auto save_button = make_button("settings_dialog:save_button"sv, c.get().style().get_color_special());
	auto cancel_button = make_button("settings_dialog:cancel_button"sv, c.get().style().get_color_primary());

	// Helper to create a gap with the standard vertical spacing
	auto make_vert_gap = [&c]() {
		return m::gap(c, {.layout_params{.dims = {ruis::dim::fill, c.get().style().get_len_gap()}}});
	};

	// Helper to create a horizontal gap (used to separate the dialog buttons)
	auto make_hori_gap = [&c]() {
		return m::gap(c, {.layout_params{.dims = {c.get().style().get_len_gap(), ruis::dim::fill}}});
	};

	// The text field for entering the day flip time.
	// Initialized with the current value from the settings storage.
	// (3:00 in the morning is the default time to start a new day food log.)
	auto day_flip_time_field = m::labeled_text_field(
		c,
		{
			.layout_params{.dims = {ruis::dim::fill, ruis::dim::min}},
			.params{
						   .label{.string = c.get().localization.get().get("settings_dialog:day_flip_time"sv)},
						   .text_input{.specific{.hint = c.get().localization.get().get("settings_dialog:day_flip_time_hint"sv)}}
			}
    },
		ruis::string(format_day_flip_time(application::inst().settings.get().day_flip_minutes))
	);

	// The Cancel button closes the dialog.
	cancel_button.get().click_handler = [](ruis::push_button& b) {
		b.get_ancestor<ruis::touch::dialog>().close();
	};

	// The Save button saves the entered day flip time to the settings storage
	// and closes the dialog. If the entered value is invalid, the dialog stays open.
	save_button.get().click_handler = [day_flip_time_field](ruis::push_button& b) {
		const auto& text_input = day_flip_time_field.get().get_text_input();
		auto text = utki::to_utf8(text_input.get_string().get());

		uint32_t day_flip_minutes;
		try {
			day_flip_minutes = parse_day_flip_time(text);
		} catch (const std::invalid_argument&) {
			// The input is not a valid time of day. Leave the dialog open.
			return;
		}

		auto s = application::inst().settings.get();
		s.day_flip_minutes = day_flip_minutes;
		application::inst().settings.set(s);

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
				c.get().localization.get().get("settings_dialog:title"sv)
			),
			make_vert_gap(),
			std::move(day_flip_time_field),
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

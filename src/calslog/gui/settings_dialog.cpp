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
#include <ruis/widget/button/touch/selection_box.hpp>
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

using namespace calslog;

namespace {
// Formats the time of day, given as minutes after midnight, as "H:MM".
std::u32string format_day_flip_time(uint32_t minutes)
{
	char buf[32];
	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
	std::snprintf(buf, sizeof buf, "%u:%02u", minutes / 60, minutes % 60);
	return utki::to_utf32(buf);
}

// Parses the time of day, given as an "H:MM"/"HH:MM" or "H.MM"/"HH.MM" string,
// and returns it as minutes after midnight.
// The hours part must consist of 1 or 2 digits (0-23), the separator can be
// either ':' or '.', and the minutes part must consist of exactly 2 digits
// (00-59), e.g. "3:00", "3.00" or "23:59".
// Throws std::invalid_argument if the input is not a valid time of day.
uint32_t parse_day_flip_time(std::string_view str)
{
	try {
		utki::string_parser p(str);
		p.skip_whitespaces();

		auto hours_view_size = p.get_view().size();
		auto hours = p.read_number<uint32_t>();
		auto hours_digits = hours_view_size - p.get_view().size();

		// The separator is either ':' or '.'
		// (peek_char() throws if the string has ended)
		auto sep = p.peek_char();
		if (sep != ':' && sep != '.') {
			throw std::invalid_argument("invalid time of day");
		}
		p.read_char();

		auto minutes_view_size = p.get_view().size();
		auto minutes = p.read_number<uint32_t>();
		auto minutes_digits = minutes_view_size - p.get_view().size();

		p.skip_whitespaces();

		if (hours_digits > 2 || minutes_digits != 2 || hours > 23 || minutes > 59 || !p.get_view().empty()) {
			throw std::invalid_argument("invalid time of day");
		}
		return hours * 60 + minutes;
	} catch (...) {
		// std::string_parser also throws std::invalid_argument on malformed input
		throw std::invalid_argument("invalid time of day");
	}
}

// Returns true if the given string is a valid time of day string.
bool is_valid_day_flip_time(std::string_view str)
{
	try {
		parse_day_flip_time(str);
	} catch (const std::invalid_argument&) {
		return false;
	}
	return true;
}

// Returns true if the given string is a prefix of a valid day flip time string,
// i.e. if it can be extended to a string in the "H:MM"/"HH:MM" format
// (or the same with '.' instead of ':') with hours 0-23 and minutes 00-59.
bool is_day_flip_time_prefix(std::u32string_view str)
{
	size_t i = 0;

	// The hours part: up to 2 digits.
	uint32_t hours = 0;
	size_t hours_digits = 0;
	while (i < str.size()) {
		const auto c = str[i];
		if (c < '0' || c > '9') {
			break;
		}
		hours = hours * 10 + (c - '0');
		++hours_digits;
		if (hours_digits > 2) {
			return false;
		}
		++i;
	}

	if (hours_digits == 2 && hours > 23) {
		// Cannot be extended to a valid hours value.
		return false;
	}

	if (i >= str.size()) {
		// No separator yet.
		return true;
	}

	if (str[i] != ':' && str[i] != '.') {
		return false;
	}
	if (hours_digits == 0) {
		// The separator cannot be the first character.
		return false;
	}
	++i;

	// The minutes part: up to 2 digits.
	uint32_t minutes = 0;
	size_t minutes_digits = 0;
	while (i < str.size()) {
		const auto c = str[i];
		if (c < '0' || c > '9') {
			return false;
		}
		minutes = minutes * 10 + (c - '0');
		++minutes_digits;
		if (minutes_digits > 2) {
			return false;
		}
		++i;
	}

	if (minutes_digits == 2) {
		return minutes <= 59;
	}
	if (minutes_digits == 1) {
		// A one-digit prefix can only be extended to 00-59 if it is 0-5.
		return minutes <= 5;
	}
	return true; // "H:" or "HH:" can still be completed.
}

// Input filter for the day flip time text field.
// Rejects an input edit if the resulting text is not a prefix of a valid time string,
// so the user cannot enter more than 2 digits in the hours or minutes part,
// more than one separator, or characters that cannot be part of a valid time.
bool day_flip_time_input_filter(
	std::u32string_view original, //
	size_t replace_start, //
	size_t replace_end, //
	std::u32string_view to_insert
)
{
	auto result = std::u32string(original.substr(0, replace_start));
	result.append(to_insert);
	result.append(original.substr(replace_end));
	return is_day_flip_time_prefix(result);
}
} // namespace

namespace {
// List provider for the language selection box.
// Shows the native names of the available UI languages.
class language_selection_box_provider : public ruis::list_provider
{
	utki::shared_ref<ruis::widget> make_widget(size_t index, bool is_highlighted) const
	{
		const auto& lang_mapping = settings_model::language_id_to_name_mapping;

		utki::assert(index < lang_mapping.size(), SL);

		auto lang_name = lang_mapping.at(index).second;

		auto& c = this->context;

		return m::text(
			c, //
			{
				.params{
					.color = is_highlighted ?
						c.get().style().get_color_text_special() :
						c.get().style().get_color_text()
				}
			}, //
			std::u32string(lang_name)
		);
	}

public:
	language_selection_box_provider(utki::shared_ref<ruis::context> context) :
		list_provider(std::move(context))
	{}

	size_t count() const noexcept override
	{
		return settings_model::language_id_to_name_mapping.size();
	}

	utki::shared_ref<ruis::widget> get_widget(size_t index) override
	{
		return this->make_widget(index, false);
	}

	utki::shared_ref<ruis::widget> get_highlighted_widget(size_t index) override
	{
		return this->make_widget(index, true);
	}
};
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
						   .text_input{
						   .specific{
							   .hint = c.get().localization.get().get("settings_dialog:day_flip_time_hint"sv), //
							   .filter = day_flip_time_input_filter
						   }
					   }
			}
    },
		ruis::string(format_day_flip_time(application::inst().settings.get().day_flip_minutes))
	);

	// The Cancel button closes the dialog.
	cancel_button.get().click_handler = [](ruis::push_button& b) {
		b.get_ancestor<ruis::touch::dialog>().close();
	};

	// The Save button is enabled only if the entered time is a valid time string.
	// NOTE: capture the shared references by value, the handlers outlive this function.
	auto update_save_button_state = [save_button](const ruis::text_string_widget& tw) {
		save_button.get().set_enabled(is_valid_day_flip_time(utki::to_utf8(tw.get_string().get())));
	};

	auto& text_input = day_flip_time_field.get().get_text_input();
	text_input.text_change_handler = [update_save_button_state](ruis::text_string_widget& tw) {
		update_save_button_state(tw);
	};

	// Set the initial state of the Save button based on the loaded value.
	update_save_button_state(text_input);

	// The Save button saves the entered day flip time to the settings storage
	// and closes the dialog. It is disabled (and thus cannot be clicked)
	// if the entered value is not a valid time string.
	save_button.get().click_handler = [day_flip_time_field](ruis::push_button& b) {
		const auto& ti = day_flip_time_field.get().get_text_input();
		auto text = utki::to_utf8(ti.get_string().get());
		auto day_flip_minutes = parse_day_flip_time(text);

		auto s = application::inst().settings.get();
		s.day_flip_minutes = day_flip_minutes;
		application::inst().settings.set(s);

		b.get_ancestor<ruis::touch::dialog>().close();
	};

	// The language selection box. Selecting a language saves it to the settings
	// and immediately reloads the whole UI with the new localization.
	// clang-format off
	auto language_selection_box = m::selection_box(c,
		{
			.layout_params{
				.dims = {ruis::dim::fill, ruis::dim::min}
			},
			.params{
				.selection_box{
					.list{
						.provider = utki::make_shared<language_selection_box_provider>(c)
					}
				},
				.specific{
					.title = c.get().localization.get().get("settings_dialog:language"sv)
				}
			}
		}
	);
	// clang-format on

	language_selection_box.get().selection_handler = [](ruis::selection_box& sb) {
		auto sel = sb.get_selection();

		// save the language to the settings storage
		{
			auto& ss = application::inst().settings;
			auto s = ss.get();

			utki::assert(sel < settings_model::language_id_to_name_mapping.size(), SL);
			s.cur_language_index = sel;

			ss.set(s);
		}

		// reload the ui with the new localization
		sb.context.get().post_to_ui_thread([sel]() {
			application::inst().load_language(sel);
		});
	};

	{
		const auto& s = application::inst().settings.get();
		utki::assert(s.cur_language_index < settings_model::language_id_to_name_mapping.size(), SL);
		language_selection_box.get().set_selection(s.cur_language_index);
	}

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
			std::move(language_selection_box),
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

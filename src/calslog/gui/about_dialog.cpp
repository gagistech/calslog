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

#include "about_dialog.hpp"

#include <string_view>

#include <ruis/widget/button/impl/rectangle_push_button.hpp>
#include <ruis/widget/group/overlay.hpp>
#include <ruis/widget/group/touch/dialog.hpp>
#include <ruis/widget/label/gap.hpp>
#include <ruis/widget/label/text.hpp>
#include <ruis/widget/widget.hpp>
#include <utki/shared.hpp>
#include <utki/unicode.hpp>

#include "../../version.hpp"
#include "style.hpp"

using namespace std::string_view_literals;

namespace calslog {

void show_about_dialog(ruis::widget& owner_widget)
{
	auto& c = owner_widget.context;

	auto& olay = owner_widget.get_ancestor<ruis::overlay>();

	// The OK button closes the dialog.
	// clang-format off
	auto ok_button = m::rectangle_push_button(c,
		{
			.layout_params{
				.dims = {ruis::dim::fill, ruis::dim::min},
				.weight = 1,
				.align = {ruis::align::front, ruis::align::back}
			},
			.params{
				.rectangle_button{
					.specific{
						.unpressed_color = c.get().style().get_color_primary()
					}
				}
			}
		},
		{
			m::text(c, {}, c.get().localization.get().get("ok_button"sv))
		}
	);
	// clang-format on

	ok_button.get().click_handler = [](ruis::push_button& b) {
		b.get_ancestor<ruis::touch::dialog>().close();
	};

	// Helper to create a gap with the standard vertical spacing
	auto make_vert_gap = [&c]() {
		return m::gap(c, {.layout_params{.dims = {ruis::dim::fill, c.get().style().get_len_gap()}}});
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
				c.get().localization.get().get("about_dialog:title"sv)
			),
			make_vert_gap(),
			m::text(c, {}, U"calslog"),
			make_vert_gap(),
			m::text(c,
				{},
				c.get().localization.get().get("about_dialog:version"sv).format({utki::to_utf32(program_version)})
			),
			make_vert_gap(),
			m::text(c, {}, c.get().localization.get().get("about_dialog:description"sv)),
			make_vert_gap(),
			m::text(c, {}, c.get().localization.get().get("about_dialog:license"sv)),
			make_vert_gap(),
			m::text(c, {}, c.get().localization.get().get("about_dialog:copyright"sv)),
			make_vert_gap(),
			std::move(ok_button)
		}
	);
	// clang-format on

	// Show the dialog
	c.get().post_to_ui_thread([olay = utki::make_shared_from(olay), dialog]() {
		olay.get().push_back(dialog);
	});
}

} // namespace calslog

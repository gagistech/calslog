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

#include "delete_confirm_dialog.hpp"

#include <functional>
#include <string_view>

#include <ruis/widget/button/impl/rectangle_push_button.hpp>
#include <ruis/widget/group/overlay.hpp>
#include <ruis/widget/group/touch/dialog.hpp>
#include <ruis/widget/label/gap.hpp>
#include <ruis/widget/label/text.hpp>
#include <ruis/widget/widget.hpp>
#include <utki/shared.hpp>

#include "style.hpp"

using namespace std::string_view_literals;

namespace calslog {

void show_delete_confirm_dialog(ruis::widget& owner_widget, std::function<void()> on_confirm)
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

	// Create the No and Yes buttons. The No button is of the special color.
	auto no_button = make_button("delete_confirm_dialog:no_button"sv, c.get().style().get_color_special());
	auto yes_button = make_button("delete_confirm_dialog:yes_button"sv, c.get().style().get_color_primary());

	// The No button closes the dialog without doing anything.
	no_button.get().click_handler = [](ruis::push_button& b) {
		b.get_ancestor<ruis::touch::dialog>().close();
	};

	// The Yes button invokes the confirm callback and closes the dialog.
	yes_button.get().click_handler = [on_confirm](ruis::push_button& b) {
		on_confirm();
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
				c.get().localization.get().get("delete_confirm_dialog:title"sv)
			),
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
					std::move(no_button),
					make_hori_gap(),
					std::move(yes_button)
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

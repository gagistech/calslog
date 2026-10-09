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

#include "context_menu.hpp"

#include <ruis/widget/container.hpp>
#include <ruis/widget/group/context_menu.hpp>
#include <ruis/widget/group/touch/context_menu.hpp>
#include <ruis/widget/label/gap.hpp>
#include <ruis/widget/label/image.hpp>
#include <ruis/widget/label/padding.hpp>
#include <ruis/widget/label/text.hpp>
#include <utki/shared.hpp>

#include "delete_confirm_dialog.hpp"
#include "style.hpp"

using namespace std::string_view_literals;

namespace calslog {

namespace {
// A ruis::list_provider which provides the widgets of a context menu whose items
// are given as a list of (icon, wording) pairs. Each item is a row with an icon
// on the left, a gap, and a text label, all wrapped with some padding. The icon
// is a shared ruis resource (its resource id is given per item) and the label is
// a localized wording (a localization key, not a string snapshot) so that it is
// re-resolved against the current localization on reload (e.g. after a language
// change). This single provider backs all the context menus in the app.
class context_menu_provider : public ruis::list_provider
{
public:
	context_menu_provider(
		const utki::shared_ref<ruis::context>& context, //
		std::vector<context_menu_item> items
	) :
		ruis::list_provider(context),
		items(std::move(items))
	{}

	size_t count() const noexcept override
	{
		return this->items.size();
	}

	utki::shared_ref<ruis::widget> get_widget(size_t index) const override
	{
		const auto& item = this->items[index];
		const auto& style = this->context.get().style();
		// clang-format off
		return m::padding(this->context,
			{
				.layout_params{
					.dims = {ruis::dim::max, ruis::dim::min}
				},
				.params{
					.container{
						.layout = ruis::layout::pile
					},
					.specific{
						.borders = {
							ruis::length::make_pp(12), // left
							ruis::length::make_pp(6), // top
							ruis::length::make_pp(12), // right
							ruis::length::make_pp(6) // bottom
						}
					}
				}
			},
			{
				// Row: icon on the left, a gap, and the text
				m::row(this->context,
					{
						.layout_params{
							.dims = {ruis::dim::max, ruis::dim::min}
						}
					},
					{
						// Icon on the left; its height matches the text height
						m::image(this->context,
							{
								.layout_params{
									.dims = {ruis::dim::min, ruis::dim::fill},
									.align = {ruis::align::front, ruis::align::center}
								},
								.params{
									.color = style.get_color_text(),
									.specific{
										.source = item.icon.get(),
										.keep_aspect_ratio = true
									}
								}
							}
						),
						// Gap between the icon and the text
						m::gap(this->context,
							{
								.layout_params{
									.dims = {style.get_len_gap(), ruis::dim::min}
								}
							}
						),
						// Item text, left-aligned in the remaining space
						m::text(
							this->context, //
							{
								.layout_params{
									.dims = {ruis::dim::min, ruis::dim::min},
									.weight = 1,
									.align = {ruis::align::front, ruis::align::center}
								}
							}, //
							item.wording
						)
					}
				)
			}
		);
		// clang-format on
	}

private:
	std::vector<context_menu_item> items;
};
} // namespace

void show_context_menu(
	ruis::widget& anchor, //
	std::vector<context_menu_item> items, //
	std::function<void(size_t index)> on_item_click
)
{
	auto& c = anchor.context;

	// Create the context menu widget holding the given menu items.
	// clang-format off
	auto menu = ruis::touch::make::context_menu(c,
		{
			.layout_params{}, //
			.widget{}, //
			.params{
				.list{
					.provider = utki::make_unique<context_menu_provider>(c, std::move(items))
				}
			}
		}
	);
	// clang-format on

	// When an item is clicked, invoke the callback. The context menu widget closes
	// itself right after this handler returns.
	menu.get().on_item_click = std::move(on_item_click);

	// Show the menu near the anchor widget, on the nearest overlay.
	ruis::show_context_menu(anchor, std::move(menu));
}

void show_item_context_menu(
	ruis::widget& anchor, //
	std::function<void()> on_edit, //
	std::function<void()> on_delete
)
{
	auto& c = anchor.context;
	// "Delete" does not delete immediately: it shows a confirmation dialog and
	// performs the deletion only when the user confirms it.
	show_context_menu(
		anchor, //
		{
			{  c.get().loader().load<ruis::res::image>("ruis_img_edit"sv),
			 c.get().localization.get().get("context_menu:edit"sv)  },
			{c.get().loader().load<ruis::res::image>("ruis_img_delete"sv),
			 c.get().localization.get().get("context_menu:delete"sv)}
    }, //
		[anchor = utki::make_shared_from(anchor), on_edit, on_delete](size_t index) {
			if (index == 0) {
				on_edit();
			} else {
				show_delete_confirm_dialog(anchor.get(), on_delete);
			}
		}
	);
}

} // namespace calslog

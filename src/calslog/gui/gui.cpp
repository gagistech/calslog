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

#include "gui.hpp"

#include <ruis/widget/button/touch/tab_button.hpp>
#include <ruis/widget/button/impl/image_push_button.hpp>
#include <ruis/widget/group/overlay.hpp>
#include <ruis/widget/group/touch/tabbed_book.hpp>
#include <ruis/widget/label/text.hpp>
#include <ruis/widget/label/padding.hpp>
#include <ruis/widget/label/rectangle.hpp>

#include "today_page.hpp"
#include "history_page.hpp"
#include "foods_page.hpp"
#include "style.hpp"

using namespace std::string_literals;
using namespace std::string_view_literals;

using namespace ruis::length_literals;

using namespace calslog;

namespace{
constexpr auto tab_bar_height = 60_pp;
constexpr auto top_bar_height = 40_pp;
constexpr auto top_bar_border = 16_pp;
constexpr auto settings_button_size = 24_pp;
}

namespace {
utki::shared_ref<ruis::tabbed_book> make_tabbed_book(utki::shared_ref<ruis::context> c,//
	ruis::layout::parameters layout_params
)
{
	auto today_page = make_today_page(c);

	// clang-format off
	auto tabbed_book = m::tabbed_book(c,
		{
			.layout_params = std::move(layout_params)
		},
		{
			{
				m::tab_button(c,
					{
						.layout_params{
							.dims = {ruis::dim::fill, ruis::dim::fill},
							.weight = 1
						}
					},
					c.get().localization.get().get("tabs:foods"sv)
				),
				make_foods_page(c)
			},
			{
				m::tab_button(c,
					{
						.layout_params{
							.dims = {ruis::dim::fill, tab_bar_height},
							.weight = 1
						},
						.image_params{
							.img = c.get().loader().load<ruis::res::image>("img_today"sv),
							.keep_aspect_ratio = true
						}
					},
					c.get().localization.get().get("tabs:today"sv)
				),
				today_page
			},
			{
				m::tab_button(c,
					{
						.layout_params{
							.dims = {ruis::dim::fill, ruis::dim::fill},
							.weight = 1
						}
					},
					c.get().localization.get().get("tabs:history"sv)
				),
				make_history_page(c)
			}
		}
	);
	// clang-format on

	// Activate Today page (second tab, index 1) by default
	today_page.get().activate();

	return tabbed_book;
}

utki::shared_ref<ruis::widget> make_top_bar(utki::shared_ref<ruis::context> c)
{
	// Get colors from style
	auto text_color = c.get().style().get_color_text();
	auto panel_color = c.get().style().get_color_panel();

	// Create background panel
	auto panel = m::rectangle(c,
		{
			.layout_params = {
				.dims = {ruis::dim::fill, top_bar_height}
			},
			.widget_params = {},
			.container_params = {},
			.padding_params = {},
			.color_params = {
				.color = panel_color
			},
			.rectangle_params = {}
		}
	);

	// Create title text - centered
	auto title = m::text(c,
		{
			.layout_params = {
				.dims = {ruis::dim::min, ruis::dim::min},
				.weight = 1,
				.align = {ruis::align::center, ruis::align::center}
			},
			.widget_params = {},
			.color_params = {
				.color = text_color
			},
			.text_params = {
				.font_size = 20_pp,
				.font_face = c.get().loader().load<ruis::res::font>("ruis_fnt_normal"sv)
			}
		},
		U"calslog"
	);

	// Create settings button with cog image - fixed size on right
	auto settings_btn = m::padding(c,
		{
			.layout_params = {
				.dims = {settings_button_size, settings_button_size}
			},
			.widget_params = {},
			.container_params = {},
			.padding_params = {
				.borders = {0_pp, 16_pp, 0_pp, 0_pp}  // right margin = 16pp
			}
		},
		{
			ruis::make::image_push_button(c,
				{
					.layout_params = {
						.dims = {ruis::dim::fill, ruis::dim::fill}
					},
					.widget_params = {},
					.button_params = {},
					.blending_params = {
						.enabled = true
					},
					.image_params = {
						.keep_aspect_ratio = true
					},
					.image_button_params = {
						.unpressed_image = c.get().loader().load<ruis::res::image>("img_cog"sv),
						.pressed_image = nullptr
					}
				}
			)
		}
	);

	// Create row with left spacer, title, and button
	ruis::widget_list row_children;
	row_children.push_back(m::rectangle(c,
		{
			.layout_params = {
				.dims = {ruis::dim::fill, ruis::dim::fill},
				.weight = 1
			},
			.widget_params = {},
			.container_params = {},
			.padding_params = {},
			.color_params = {
				.color = 0x00000000  // transparent
			},
			.rectangle_params = {}
		}
	));
	row_children.push_back(std::move(title));
	row_children.push_back(std::move(settings_btn));

	auto row_widget = m::row(c,
		{
			.layout_params = {
				.dims = {ruis::dim::fill, ruis::dim::fill}
			},
			.widget_params = {}
		},
		std::move(row_children)
	);

	// Stack panel as background, then row with title and button on top
	return m::pile(c,
		{
			.layout_params = {
				.dims = {ruis::dim::fill, top_bar_height}
			},
			.widget_params = {}
		},
		{
			std::move(panel),
			std::move(row_widget)
		}
	);
}
} // namespace

utki::shared_ref<ruis::widget> calslog::make_root_widget(utki::shared_ref<ruis::context> c)
{
	// clang-format off
	return m::overlay(c,
		{},
		{
			m::column(c,
				{
					.layout_params = {
						.dims = {ruis::dim::fill, ruis::dim::fill}
					},
					.widget_params = {}
				},
				{
					make_top_bar(c),
					make_tabbed_book(c,
						ruis::layout::parameters{
							.dims = {ruis::dim::fill, ruis::dim::fill},
							.weight = 1
						}
					)
				}
			)
		}
	);
	// clang-format on
}

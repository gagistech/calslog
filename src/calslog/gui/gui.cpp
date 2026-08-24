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

#include <ruis/widget/button/impl/ellipse_push_button.hpp>
#include <ruis/widget/button/impl/rectangle_push_button.hpp>
#include <ruis/widget/button/touch/tab_button.hpp>
#include <ruis/widget/group/overlay.hpp>
#include <ruis/widget/group/touch/tabbed_book.hpp>
#include <ruis/widget/label/image.hpp>
#include <ruis/widget/label/padding.hpp>
#include <ruis/widget/label/rectangle.hpp>
#include <ruis/widget/label/text.hpp>

#include "foods_page.hpp"
#include "history_page.hpp"
#include "style.hpp"
#include "today_page.hpp"

using namespace std::string_literals;
using namespace std::string_view_literals;

using namespace ruis::length_literals;

using namespace calslog;

namespace {
constexpr auto tab_bar_height = 60_pp;
constexpr auto top_bar_height = 45_pp;
} // namespace

namespace {
utki::shared_ref<ruis::tabbed_book> make_tabbed_book(
	utki::shared_ref<ruis::context> c, //
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
	// clang-format off
	return m::rectangle(c,
		{
			.layout_params = {
				.dims = {ruis::dim::fill, top_bar_height}
			},
			.container_params = {
				.layout = ruis::layout::pile
			},
			.padding_params = {
				.borders = c.get().style().get_len_gap()
			},
			.color_params = {
				.color = c.get().style().get_color_panel()
			}
		},
		{
			m::text(c,
				{
					.text_params = {
						.font_size = c.get().style().get_font_size_title()
					}
				},
				U"calslog"
			),
			// Settings button
			m::ellipse_push_button(c,
				{
					.layout_params = {
						.dims = {ruis::dim::min, ruis::dim::fill},
						.align = {ruis::align::back, ruis::align::center}
					},
					.container_params = {
						.layout = ruis::layout::pile
					},
					.ellipse_button_params = {
						.unpressed_color = c.get().style().get_color_panel()
					}
				},
				{
					ruis::make::image(c,
						{
							.layout_params = {
								.dims = {ruis::dim::min, ruis::dim::fill}
							},
							.image_params = {
								.img = c.get().loader().load<ruis::res::image>("img_cog"sv),
								.keep_aspect_ratio = true
							}
						}
					)
				}
			)
		}
	);
	// clang-format on
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
					}
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

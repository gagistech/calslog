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
#include <ruis/widget/group/overlay.hpp>
#include <ruis/widget/group/touch/tabbed_book.hpp>

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
}

namespace {
ruis::widget_list make_root_widget_structure(utki::shared_ref<ruis::context> c)
{
	// clang-format off
	auto today_page = make_today_page(c);
	auto foods_page = make_foods_page(c);
	auto history_page = make_history_page(c);

	auto tabbed_book = ruis::touch::make::tabbed_book(c,
		{
			.layout_params = {
				.dims = {ruis::dim::fill, ruis::dim::fill}
			}
		},
		{
			{
				ruis::touch::make::tab_button(c,
					{
						.layout_params{
							.dims = {ruis::dim::fill, ruis::dim::fill},
							.weight = 1
						}
					},
					c.get().localization.get().get("tabs:foods"sv)
				),
				foods_page
			},
			{
				ruis::touch::make::tab_button(c,
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
				ruis::touch::make::tab_button(c,
					{
						.layout_params{
							.dims = {ruis::dim::fill, ruis::dim::fill},
							.weight = 1
						}
					},
					c.get().localization.get().get("tabs:history"sv)
				),
				history_page
			}
		}
	);

	// Activate Today page (second tab, index 1) by default
	today_page.get().activate();

	return {std::move(tabbed_book)};
	// clang-format on
}
} // namespace

utki::shared_ref<ruis::widget> calslog::make_root_widget(utki::shared_ref<ruis::context> c)
{
	// clang-format off
	return m::overlay(c,
		{},
		make_root_widget_structure(c)
	);
	// clang-format on
}

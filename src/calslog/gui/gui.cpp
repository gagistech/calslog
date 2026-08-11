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

namespace {
ruis::widget_list make_root_widget_structure(utki::shared_ref<ruis::context> c)
{
	// clang-format off
	return {
		ruis::touch::make::tabbed_book(c,
			{
				.layout_params = {
					.dims = {ruis::dim::fill, ruis::dim::fill}
				}
			},
			{
				{
					ruis::touch::make::tab_button(c,
						{
							.layout_params = {
								.dims = {ruis::dim::fill, 60_pp},
								.weight = 1
							}
						},
						U"Today"s
					),
					make_today_page(c)
				},
				{
					ruis::touch::make::tab_button(c,
						{
							.layout_params = {
								.dims = {ruis::dim::fill, 60_pp},
								.weight = 1
							}
						},
						U"Foods"s
					),
					make_foods_page(c)
				},
				{
					ruis::touch::make::tab_button(c,
						{
							.layout_params = {
								.dims = {ruis::dim::fill, 60_pp},
								.weight = 1
							}
						},
						U"History"s
					),
					make_history_page(c)
				}
			}
		)
	};
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

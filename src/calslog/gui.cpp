/*
calslog - wall calslog control example GUI project

Copyright (C) 2025 Gagistech Oy <gagistechoy@gmail.com>

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

#include <ruis/widget/group/overlay.hpp>

#include "style.hpp"

using namespace std::string_literals;
using namespace std::string_view_literals;

using namespace ruis::length_literals;

using namespace calslog;

namespace {
utki::shared_ref<ruis::widget> make_root_widget_structure(utki::shared_ref<ruis::context> c)
{
	// clang-format off
	return m::container(c,
		{
			.layout_params = {
				.dims = {ruis::dim::fill, ruis::dim::fill}
			},
			.container_params = {
				.layout = ruis::layout::column
			}
		},
		{
		}
	);
	// clang-format on
}
} // namespace

utki::shared_ref<ruis::widget> calslog::make_root_widgets(utki::shared_ref<ruis::context> c)
{
	// clang-format off
	return m::overlay(c,
		{},
		{
			make_root_widget_structure(c)
		}
	);
	// clang-format on
}

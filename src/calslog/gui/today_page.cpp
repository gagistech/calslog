/*
calslog - Calories logging mobile applcation

Copyright (C) 2026-2026 Gagistech Oy <gagisechoy@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

/* ================ LICENSE END ================ */

#include "today_page.hpp"

#include <ruis/widget/group/touch/list.hpp>
#include <ruis/widget/label/gap.hpp>
#include <ruis/widget/label/padding.hpp>
#include <ruis/widget/label/text.hpp>
#include <utki/string.hpp>

#include "style.hpp"

using namespace std::string_literals;

using namespace ruis::length_literals;

namespace calslog {

namespace {
class today_page_provider : public ruis::list_provider
{
	std::vector<std::u32string> items = {
		U"Apple",
		U"Banana",
		U"Orange",
		U"Milk",
		U"Bread",
		U"Cheese",
		U"Eggs",
		U"Chicken",
		U"Rice",
		U"Pasta",
		U"Tomato",
		U"Cucumber"
	};

public:
	today_page_provider(utki::shared_ref<ruis::context> context) :
		ruis::list_provider(std::move(context))
	{}

	size_t count() const noexcept override
	{
		return this->items.size();
	}

	utki::shared_ref<ruis::widget> get_widget(size_t index) override
	{
		// clang-format off
		return m::padding(this->context,
			{
				.layout_params{
					.dims = {ruis::dim::fill, ruis::dim::min}
				},
				.container_params{
					.layout = ruis::layout::row
				},
				.padding_params{
					.borders = {ruis::length::make_pp(10)}
				}
			},
			{
				m::text(this->context,
					{
						.text_params{
							.font_size = ruis::length::make_pp(20)
						}
					},
					this->items.at(index)
				),
				m::gap(this->context,
					{
						.layout_params{
							.dims = {ruis::dim::fill, ruis::dim::min}
						}
					}
				),
				m::text(this->context,
					{
						.color_params{
							.color = 0xff808080
						},
						.text_params{
							.font_size = ruis::length::make_pp(20)
						}
					},
					utki::to_utf32(utki::cat(index + 1)) + U" kcal"
				)
			}
		);
		// clang-format on
	}
};
} // namespace

namespace {
class today_page : public ruis::page, private ruis::touch::list
{
public:
	today_page(utki::shared_ref<ruis::context> context) :
		// clang-format off
		ruis::widget(std::move(context),
			{},
			{
				.clip = true
			}
		),
		// clang-format on
		ruis::page(this->context, {}),
		// clang-format off
		ruis::touch::list(this->context,
			{
				.oriented_params{
					.vertical = true
				},
				.list_params{
					.provider = utki::make_shared<today_page_provider>(this->context)
				}
			}
		)
	// clang-format on
	{}
};
} // namespace

utki::shared_ref<ruis::page> make_today_page(
	utki::shared_ref<ruis::context> context
)
{
	return utki::make_shared<today_page>(std::move(context));
}

} // namespace calslog
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

#include "today_page.hpp"

#include <ruis/widget/button/impl/rectangle_push_button.hpp>
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
class today_page : public ruis::page, //
private ruis::container
{
private:
	today_page(
		utki::shared_ref<ruis::context> context, //
		utki::shared_ref<ruis::touch::list> list_widget, //
		utki::shared_ref<ruis::rectangle_push_button> fab_button
	) :
		// clang-format off
		ruis::widget(
			std::move(context),
			{},
			{
				.clip = true
			}
		),
		// clang-format on
		ruis::page(this->context, {}),
		ruis::container(this->context,
			{
				.container_params{
					.layout = ruis::layout::pile
				}
			},
			{
				std::move(list_widget),
				m::padding(this->context,
					{
						.layout_params{
							.align = {ruis::align::back, ruis::align::back}
						},
						.padding_params{
							.borders = {16_pp} // TODO: make multiplier of gap?
						}
					},
					{
						std::move(fab_button)
					}
				)
			}
		)
	{}

public:
	today_page(utki::shared_ref<ruis::context> context) :
		today_page(
			std::move(context),
			// Create the list widget
			ruis::touch::make::list(context,
				{
					.layout_params{
						.dims = {ruis::dim::fill, ruis::dim::fill}
					},
					.oriented_params{
						.vertical = true
					},
					.list_params{
						.provider = utki::make_shared<today_page_provider>(context)
					}
				}
			),
			// Create the floating action button (FAB)
			m::rectangle_push_button(context,
				{
					.layout_params{
						.dims = {56_pp} // TODO: why this size?
					},
					.container_params{
						.layout = ruis::layout::pile
					},
					.rectangle_params{
						.corner_radii = {16_pp}
					},
					.rectangle_button_params{
						.unpressed_color = context.get().style().get_color_special()
					}
				},
				{
					// Plus sign in the center
					// TODO: use icon
					ruis::make::text(context,
						{
							.color_params{
								.color = ruis::color(0xffffffff) // White color for the plus sign
							},
							.text_params{
								.font_size = ruis::length::make_pp(28)
							}
						},
						U"+"
					)
				}
			)
		)
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
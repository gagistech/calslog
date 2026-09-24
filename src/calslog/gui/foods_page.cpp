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

#include "foods_page.hpp"

#include <ruis/widget/group/touch/list.hpp>
#include <ruis/widget/label/gap.hpp>
#include <ruis/widget/label/padding.hpp>
#include <ruis/widget/label/rectangle.hpp>
#include <ruis/widget/label/text.hpp>
#include <utki/string.hpp>

#include "../application.hpp"
#include "../model/model.hpp"

#include "style.hpp"

using namespace std::string_literals;

using namespace ruis::length_literals;

namespace calslog {

namespace {
class foods_page_provider : public ruis::list_provider
{
public:
	foods_page_provider(const utki::shared_ref<ruis::context>& context) :
		ruis::list_provider(context)
	{}

	size_t count() const noexcept override
	{
		return application::inst().model.foods.size();
	}

	utki::shared_ref<ruis::widget> get_widget(size_t index) override
	{
		const auto& food = application::inst().model.foods.at(index);

		const auto& style = this->context.get().style();
		const auto len_border = style.get_len_border();
		const auto len_gap = style.get_len_gap();
		const auto len_gap_small = style.get_len_gap_small();
		const auto color_primary = style.get_color_primary();
		const auto font_size_secondary = style.get_font_size_secondary();
		const auto color_text_secondary = style.get_color_text_secondary();

		// clang-format off
        return m::column(this->context,
            // column for item content and separator
            {
                .layout_params{
                    .dims = {ruis::dim::fill, ruis::dim::min}
                }
            },
            {
                // Item content
                m::padding(this->context,
                    {
                        .layout_params{
                            .dims = {ruis::dim::fill, ruis::dim::min}
                        },
                        .params{
                            .container{
                                .layout = ruis::layout::column
                            },
                            .specific{
                                .borders = {len_gap}
                            }
                        }
                    },
                    {
                        // Line 1: food name, default font
                        m::text(this->context,
                            {
                                .layout_params{
                                    .dims = {ruis::dim::fill, ruis::dim::min}
                                }
                            },
                            food.name
                        ),
                        m::gap(this->context,
                            {
                                .layout_params{
                                    .dims = {0_pp, len_gap_small}
                                }
                            }
                        ),
                        // Line 2: kcal/100g and mass per serving, secondary text style
                        m::text(this->context,
                            {
                                .layout_params{
                                    .align = {ruis::align::front, ruis::align::front}
                                },
                                .params{
                                    .color = color_text_secondary,
                                    .font{
                                        .size = font_size_secondary
                                    }
                                }
                            },
                            utki::to_utf32(utki::to_string(food.kcal)) + U" kcal/100g, " + utki::to_utf32(utki::to_string(food.mass)) + U" g/serving"
                        )
                    }
                ),
                // separator
                m::rectangle(this->context,
                    {
                        .layout_params{
                            .dims = {ruis::dim::fill, len_border}
                        },
                        .params{
                            .specific{
                                .fill_color = color_primary
                            }
                        }
                    }
                )
            }
        );
		// clang-format on
	}
};
} // namespace

namespace {
class foods_page :
	public ruis::page, //
	private ruis::touch::list
{
public:
	foods_page(const utki::shared_ref<ruis::context>& context) :
		// clang-format off
		ruis::widget(context,
			{},
			{
				.clip = true
			}
		),
		// clang-format on
		ruis::page(context, {}),
		// clang-format off
		ruis::touch::list(context,
			{
				.params{
                    .specific{
                        .provider = utki::make_shared<foods_page_provider>(context)
                    }
                }
			}
		)
	// clang-format on
	{}
};
} // namespace

utki::shared_ref<ruis::page> make_foods_page(const utki::shared_ref<ruis::context>& context)
{
	return utki::make_shared<foods_page>(context);
}

} // namespace calslog
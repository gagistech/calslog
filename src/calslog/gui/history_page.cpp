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

#include "history_page.hpp"

#include <cstdio>

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
using namespace std::string_view_literals;

using namespace ruis::length_literals;

namespace calslog {

namespace {
std::u32string make_date_string(std::chrono::year_month_day d)
{
	char buf[16];
	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
	std::snprintf(
		buf, //
		sizeof buf, //
		"%04d-%02u-%02u", //
		static_cast<int>(d.year()), //
		static_cast<unsigned>(d.month()), //
		static_cast<unsigned>(d.day())
	);
	return utki::to_utf32(buf);
}
} // namespace

namespace {
class history_page_provider : public ruis::list_provider
{
public:
	history_page_provider(const utki::shared_ref<ruis::context>& context) :
		ruis::list_provider(context)
	{}

	size_t count() const noexcept override
	{
		return application::inst().model.history.size();
	}

	utki::shared_ref<ruis::widget> get_widget(size_t index) override
	{
		const auto& day = application::inst().model.history.at(index);

		const auto& style = this->context.get().style();
		const auto len_border = style.get_len_border();
		const auto len_gap = style.get_len_gap();
		const auto len_gap_small = style.get_len_gap_small();
		const auto color_primary = style.get_color_primary();
		const auto font_size_secondary = style.get_font_size_secondary();
		const auto color_text_secondary = style.get_color_text_secondary();

		const std::u32string weight_str = utki::to_utf32(day.get_weight_string());

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
                        // Line 1: date (left) and total calories (right), default font
                        m::row(this->context,
                            {
                                .layout_params{
                                    .dims = {ruis::dim::fill, ruis::dim::min}
                                }
                            },
                            {
                                m::text(this->context,
                                    {
                                        .layout_params{
                                            .weight = 1,
                                            .align = {ruis::align::front, ruis::align::center}
                                        }
                                    },
                                    make_date_string(day.date)
                                ),
                                m::text(this->context,
                                    {},
                                    this->context.get().localization.get()
                                        .get("kcal"sv)
                                        .format({utki::to_utf32(std::to_string(day.calc_total_kcal()))})
                                )
                            }
                        ),
                        m::gap(this->context,
                            {
                                .layout_params{
                                    .dims = {0_pp, len_gap_small}
                                }
                            }
                        ),
                        // Line 2: weight, secondary text style
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
                            this->context.get().localization.get()
                                .get("weight"sv)
                                .format({weight_str})
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
class history_page : public ruis::page, private ruis::touch::list
{
public:
	history_page(const utki::shared_ref<ruis::context>& context) :
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
                        .provider = utki::make_shared<history_page_provider>(context)
                    }
                }
			}
		)
	// clang-format on
	{}
};
} // namespace

utki::shared_ref<ruis::page> make_history_page(const utki::shared_ref<ruis::context>& context)
{
	return utki::make_shared<history_page>(context);
}

} // namespace calslog
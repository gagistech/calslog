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
#include "util.hpp"

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

	utki::shared_ref<ruis::widget> get_widget(size_t index) const override
	{
		// Show the most recent days at the top: reverse the display index into the
		// history, which is ordered oldest -> newest.
		const auto& history = application::inst().model.history;
		const auto& day = history.at(history.size() - 1 - index);

		// The top item (index 0) is the most recent day, which is today (guaranteed at
		// start-up). Show it with a localized "(today)" suffix for clarity.
		std::u32string date_label = make_date_string(day.date);
		if (index == 0) {
			date_label =
				this->context.get().localization.get().get("history_page:today"sv).format({date_label}).string();
		}

		const auto& style = this->context.get().style();
		const auto len_border = style.get_len_border();
		const auto len_gap = style.get_len_gap();
		const auto len_gap_small = style.get_len_gap_small();
		const auto color_secondary = style.get_color_secondary();
		const auto color_text = style.get_color_text();
		const auto font_size_secondary = style.get_font_size_secondary();
		const auto color_text_secondary = style.get_color_text_secondary();

		const auto kcal = make_kcal_wording(this->context.get().localization.get(), day);

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
                                    date_label
                                ),
                                // Total calories (in the "exceeded" color when the day goal is
                                // exceeded) and the goal suffix ("/XXX kcal", primary text color)
                                m::row(this->context,
                                    {
                                        .layout_params{
                                            .dims = {ruis::dim::min, ruis::dim::min}
                                        }
                                    },
                                    {
                                        m::text(this->context,
                                            {
                                                .params{
                                                    .color = kcal.exceeded ? //
                                                        style.get_color_critical() : //
                                                        color_text
                                                }
                                            },
                                            kcal.total
                                        ),
                                        m::text(this->context,
                                            {
                                                .params{
                                                    .color = color_text
                                                }
                                            },
                                            kcal.goal
                                        )
                                    }
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
                                .fill_color = color_secondary
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
class history_page :
	public ruis::page, //
	private ruis::container
{
private:
	utki::shared_ref<ruis::touch::list> list_widget;

	history_page(
		const utki::shared_ref<ruis::context>& context, //
		utki::shared_ref<ruis::touch::list> list_widget_param //
	) :
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
		ruis::container(
			context,
			{
				.params{
					.layout = ruis::layout::column
				}
			},
			{
				// The list fills the whole page
				m::pile(
					context,
					{
						.layout_params{
							.dims = {ruis::dim::fill, ruis::dim::fill},
							.weight = 1
						}
					},
					{
						list_widget_param
					}
				)
			}
		),
		// clang-format on
		list_widget(std::move(list_widget_param))
	{}

public:
	history_page(const utki::shared_ref<ruis::context>& context) :
		// clang-format off
		history_page(
			context,
			// Create the list widget
			m::list(
				context,
				{
					.layout_params{
						.dims = {ruis::dim::fill, ruis::dim::fill}
					},
					.params{
						.specific{
							.provider = utki::make_unique<history_page_provider>(context)
						}
					}
				}
			)
		)
	// clang-format on
	{
		// Refresh the list whenever the model changes (e.g. a day's totals or weight
		// is updated). Capture a weak reference to the list (not a raw pointer and
		// not 'this') so the handler stays safe even if it were to outlive the list;
		// lock it and notify the provider on each model change.
		auto list_weak = utki::make_weak(this->list_widget);
		application::inst().model.model_changed_signal.connect([list_weak]() {
			if (auto list = list_weak.lock()) {
				list->notify_model_change();
			}
		});
	}
};
} // namespace

utki::shared_ref<ruis::page> make_history_page(const utki::shared_ref<ruis::context>& context)
{
	return utki::make_shared<history_page>(context);
}

} // namespace calslog
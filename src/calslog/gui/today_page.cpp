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

#include <functional>

#include <ruis/widget/button/impl/check_box.hpp>
#include <ruis/widget/button/impl/image_push_button.hpp>
#include <ruis/widget/button/impl/rectangle_push_button.hpp>
#include <ruis/widget/group/touch/list.hpp>
#include <ruis/widget/label/gap.hpp>
#include <ruis/widget/label/image.hpp>
#include <ruis/widget/label/padding.hpp>
#include <ruis/widget/label/rectangle.hpp>
#include <ruis/widget/label/text.hpp>
#include <utki/string.hpp>

#include "../application.hpp"
#include "../model/model.hpp"

#include "log_food_dialog.hpp"
#include "style.hpp"

using namespace std::string_literals;
using namespace std::string_view_literals;

using namespace ruis::length_literals;

namespace calslog {

namespace {
class today_page_provider : public ruis::list_provider
{
public:
	today_page_provider(const utki::shared_ref<ruis::context>& context) :
		ruis::list_provider(context)
	{}

	size_t count() const noexcept override
	{
		return application::inst().model.today.entries.size();
	}

	utki::shared_ref<ruis::widget> get_widget(size_t index) override
	{
		const auto& entry = application::inst().model.today.entries.at(index);
		const uint32_t total_kcal = entry.calc_total_kcal();

		// Checkbox reflecting whether the entry is counted in the day total.
		// When toggled, the entry's enabled state in the model is updated and the
		// model's model_changed_signal is emitted so that dependent UI (the day
		// total kcal display) is refreshed.
		auto check_box_widget = m::check_box(this->context,
			{
				.button{
					.pressed = entry.enabled
				}
			}
		);
		check_box_widget.get().pressed_change_handler = [index](ruis::button& b) {
			auto& e = application::inst().model.today.entries.at(index);
			e.enabled = b.is_pressed();
			application::inst().model.model_changed_signal.emit();
		};

		const auto& style = this->context.get().style();
		const auto len_border = style.get_len_border();
		const auto len_gap = style.get_len_gap();
		const auto len_gap_small = style.get_len_gap_small();
		const auto color_primary = style.get_color_primary();
		const auto font_size_secondary = style.get_font_size_secondary();
		const auto color_text_secondary = style.get_color_text_secondary();

		// clang-format off
        return m::column(this->context,
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
                                .layout = ruis::layout::row
                            },
                            .specific{
                                .borders = {len_gap}
                            }
                        }
                    },
                    {
                        // Left column with the text lines, fills the remaining width
                        m::column(this->context,
                            {
                                .layout_params{
                                    .dims = {ruis::dim::fill, ruis::dim::min},
                                    .weight = 1
                                }
                            },
                            {
                                // Line 1: food title (left) and total calories (right), default font
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
                                            entry.name
                                        ),
                                        m::text(this->context,
                                            {},
                                            this->context.get().localization.get()
                                                .get("kcal"sv)
                                                .format({utki::to_utf32(std::to_string(total_kcal))})
                                                .string()
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
                                // Line 2: secondary text, aligned to the left
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
                                        .get("today_page:entry_detail"sv)
                                        .format({
                                            utki::to_utf32(std::to_string(entry.pcs)),
                                            utki::to_utf32(std::to_string(entry.mass)),
                                            utki::to_utf32(std::to_string(entry.kcal))
                                        })
                                        .string()
                                )
                            }
                        ),
                        // Gap before the checkbox
                        m::gap(this->context,
                            {
                                .layout_params{
                                    .dims = {ruis::dimension(len_gap), ruis::dim::min}
                                }
                            }
                        ),
                        // Checkbox on the right, vertically centered, reflects the entry's enabled state
                        std::move(check_box_widget)
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
class today_page :
	public ruis::page, //
	private ruis::container
{
private:
	utki::shared_ref<ruis::rectangle_push_button> fab_button;
	utki::shared_ref<ruis::text> total_kcal_text;
	utki::shared_ref<ruis::text> weight_text;

	today_page(
		const utki::shared_ref<ruis::context>& context, //
		utki::shared_ref<ruis::text> total_kcal_text_param, //
		utki::shared_ref<ruis::text> weight_text_param, //
		utki::shared_ref<ruis::touch::list> list_widget, //
		utki::shared_ref<ruis::rectangle_push_button> fab_button_param
	) :
		// clang-format off
        ruis::widget(
            context,
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
                // Total kcal and weight fields at the top of the page
                m::padding(
                    context,
                    {
                        .layout_params{
                            .dims = {ruis::dim::fill, ruis::dim::min}
                        },
                        .params{
                            .specific{
                                .borders = {context.get().style().get_len_gap_small()}
                            }
                        }
                    },
                    {
                        m::column(
                            context,
                            {
                                .layout_params{
                                    .dims = {ruis::dim::fill, ruis::dim::min}
                                }
                            },
                            {
                                total_kcal_text_param,
                                m::gap(context,
                                    {
                                        .layout_params{
                                            .dims = {0_pp, context.get().style().get_len_gap_small()}
                                        }
                                    }
                                ),
                                weight_text_param
                            }
                        )
                    }
                ),
                // Separator between the total kcal field and the list
                m::rectangle(context,
                    {
                        .layout_params{
                            .dims = {ruis::dim::fill, context.get().style().get_len_gap_small()}
                        },
                        .params{
                            .specific{
                                .fill_color = context.get().style().get_color_primary()
                            }
                        }
                    }
                ),
                // List and the floating action button on top of it
                m::pile(
                    context,
                    {
                        .layout_params{
                            .dims = {ruis::dim::fill, ruis::dim::fill},
                            .weight = 1
                        }
                    },
                    {
                        std::move(list_widget),
                        m::padding(
                            context,
                            {
                                .layout_params{
                                    .align = {ruis::align::back, ruis::align::back}
                                },
                                .params{
                                    .specific{
                                        .borders = {context.get().style().get_len_gap_big()}
                                    }
                                }
                            },
                            {
                                fab_button_param
                            }
                        )
                    }
                )
            }
        ),
        fab_button(fab_button_param),
        total_kcal_text(total_kcal_text_param),
        weight_text(weight_text_param)
	// clang-format on
	{}

public:
	today_page(const utki::shared_ref<ruis::context>& context) :
		today_page(
			context,
			// Total kcal field, kept as a member so it can be updated when an entry is enabled/disabled
            m::text(context,
                {},
                context.get().localization.get()
                    .get("total_kcal"sv)
                    .format({utki::to_utf32(std::to_string(application::inst().model.today.calc_total_kcal()))})
                    .string()
            ),
			// Weight field, kept as a member so it can be updated when the model changes
            m::text(context,
                {},
                context.get().localization.get()
                    .get("weight"sv)
                    .format({utki::to_utf32(application::inst().model.today.get_weight_string())})
                    .string()
            ),
			// Create the list widget
			// clang-format off
            ruis::touch::make::list(
                context,
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
            m::rectangle_push_button(
                context,
                {
                    .layout_params{
                        .dims = {56_pp}
                    },
                    .params{
                        .rectangle_button{
                            .rectangle{
                                .padding{
                                    .container{
                                        .layout = ruis::layout::pile
                                    },
                                    .specific{
                                        .borders = {14_pp}
                                    }
                                },
                                .specific{
                                    .corner_radii = {14_pp}
                                }
                            },
                            .specific{
                                .unpressed_color = context.get().style().get_color_special()
                            }
                        }
                    }
                },
                {
                    ruis::make::image(
                        context,
                        {
                            .layout_params{
                                .dims = {ruis::dim::fill}
                            },
                            .params{
                                .specific{
                                    .source = context.get().loader().load<ruis::res::image>("img_add"sv)
                                }
                            }
                        }
                    )
                }
            )
			// clang-format on
		)
	{
		// Set click handler on the FAB button
		// Capture 'this' as raw pointer and create weak_ptr inside handler,
		// because shared_from_this() doesn't work during construction.
		this->fab_button.get().click_handler = [](ruis::push_button& b) {
			show_log_food_dialog(b);
		};

		// Listen to model changes and refresh the day total kcal display.
		// No explicit disconnect is required: the model (and its
		// model_changed_signal) is a member of the application and is destroyed
		// before the GUI, so the signal can never emit into a dangling page.
		application::inst().model.model_changed_signal.connect([this]() {
			this->total_kcal_text.get().set_text(
				this->context.get().localization.get()
					.get("total_kcal"sv)
					.format({utki::to_utf32(std::to_string(application::inst().model.today.calc_total_kcal()))})
					.string()
			);
			this->weight_text.get().set_text(
				this->context.get().localization.get()
					.get("weight"sv)
					.format({utki::to_utf32(application::inst().model.today.get_weight_string())})
					.string()
			);
		});
	}
};
} // namespace

utki::shared_ref<ruis::page> make_today_page(const utki::shared_ref<ruis::context>& context)
{
	return utki::make_shared<today_page>(context);
}

} // namespace calslog
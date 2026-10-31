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

#include "day_entries_dialog.hpp"

#include <algorithm>
#include <chrono>
#include <string>
#include <string_view>

#include <ruis/widget/button/impl/rectangle_push_button.hpp>
#include <ruis/widget/group/overlay.hpp>
#include <ruis/widget/group/touch/dialog.hpp>
#include <ruis/widget/group/touch/list.hpp>
#include <ruis/widget/label/gap.hpp>
#include <ruis/widget/label/padding.hpp>
#include <ruis/widget/label/rectangle.hpp>
#include <ruis/widget/label/text.hpp>
#include <utki/string.hpp>
#include <utki/unicode.hpp>

#include "../application.hpp"
#include "../model/model.hpp"

#include "style.hpp"
#include "util.hpp"

using namespace std::string_view_literals;

using namespace ruis::length_literals;

namespace calslog {

namespace {
// A ruis::list_provider which provides the widgets of the entries list of the day
// entries dialog. It shows a snapshot of the day's entries (taken when the dialog is
// created), laid out the same way as the entries on the "today" page, but without the
// flip switch and the context menu button of those. Entries which are not counted in
// the day total are shown in the secondary text color.
class day_entries_list_provider : public ruis::list_provider
{
public:
	day_entries_list_provider(
		const utki::shared_ref<ruis::context>& context, //
		utki::shared_ref<model::day> day
	) :
		ruis::list_provider(context), //
		day(std::move(day))
	{}

	size_t count() const noexcept override
	{
		return this->day.get().entries.size();
	}

	utki::shared_ref<ruis::widget> get_widget(size_t index) const override
	{
		const auto& entry = this->day.get().entries.at(index);
		const uint32_t total_kcal = entry.calc_total_kcal();

		const auto& style = this->context.get().style();
		const auto len_border = style.get_len_border();
		const auto len_gap = style.get_len_gap();
		const auto len_gap_small = style.get_len_gap_small();
		const auto color_secondary = style.get_color_secondary();
		const auto font_size_secondary = style.get_font_size_secondary();
		const auto color_text_secondary = style.get_color_text_secondary();

		// Entries which are not counted in the day total are dimmed.
		const auto color_text = entry.enabled ? style.get_color_text() : color_text_secondary;

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
								.layout = ruis::layout::column
							},
							.specific{
								.borders = {len_gap}
							}
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
										},
										.params{ .text_string{
											.color = color_text
										 } }
									},
									entry.name
								),
								m::text(this->context,
									{
										.params{ .text_string{
											.color = color_text
										 } }
									},
									this->context.get().localization.get()
										.get("kcal"sv)
										.format({utki::to_utf32(std::to_string(total_kcal))})
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
								.params{ .text_string{
									.color = color_text_secondary,
									.font{
										.size = font_size_secondary
									}
								 } }
							},
							this->context.get().localization.get()
								.get("entry_detail"sv)
								.format({
									utki::to_utf32(utki::to_string(entry.pcs)),
									utki::to_utf32(std::to_string(entry.mass)),
									utki::to_utf32(std::to_string(entry.kcal))
								})
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

private:
	utki::shared_ref<model::day> day;
};
} // namespace

void show_day_entries_dialog(ruis::widget& owner_widget, std::chrono::year_month_day date)
{
	auto& c = owner_widget.context;

	auto& olay = owner_widget.get_ancestor<ruis::overlay>();

	// Look up the day in the model's history and take a snapshot of its data. The
	// dialog is read-only, but the snapshot keeps the shown data consistent even if
	// the model changes (e.g. the logged day flips over) while the dialog is open.
	const auto& history = application::inst().model.history;
	auto day_it = std::ranges::find_if(history, [date](const model::day& d) {
		return d.date == date;
	});
	if (day_it == history.end()) {
		return;
	}
	auto day = utki::make_shared<model::day>(*day_it);

	const auto& style = c.get().style();
	auto& loc = c.get().localization.get();

	// The dialog's title is the day's date, with the localized "(today)" suffix
	// appended for the day which is the current one.
	std::u32string title = make_date_string(day.get().date);
	if (day.get().date == application::inst().model.today().date) {
		title = loc.get("history_page:today"sv).format({title}).string();
	}

	// Helper to create a gap with the standard vertical spacing
	auto make_vert_gap = [&c]() {
		return m::gap(c, {.layout_params{.dims = {ruis::dim::fill, c.get().style().get_len_gap()}}});
	};

	// Helper to create a push button with a localized caption and the given color.
	auto make_button = [&c](std::string_view text_loc_id, ruis::styled<ruis::color> color) {
		// clang-format off
		return m::rectangle_push_button(c,
			{
				.layout_params{
					.dims = {ruis::dim::fill, ruis::dim::min},
					.weight = 1
				},
				.params{
					.rectangle_button{
						.specific{
							.unpressed_color = color
						}
					}
				}
			},
			{
				m::text(c, {}, c.get().localization.get().get(text_loc_id))
			}
		);
		// clang-format on
	};

	// The dialog is read-only, the only button closes it.
	auto ok_button = make_button("ok_button"sv, style.get_color_special());
	ok_button.get().click_handler = [](ruis::push_button& b) {
		b.get_ancestor<ruis::touch::dialog>().close();
	};

	// The day's total calories (in the "exceeded" color when the day goal is exceeded)
	// and the goal suffix, worded and colored the same way as on the "today" page.
	const auto kcal = make_kcal_wording(loc, day.get());

	// clang-format off
	auto total_text = m::text(c,
		{
			.params{ .text_string{
				.color = kcal.exceeded ? //
					style.get_color_critical() : //
					style.get_color_text_special(),
				.specific{
					.style = ruis::res::font::style::bold
				}
			 } }
		},
		kcal.total
	);

	auto goal_text = m::text(c,
		{
			.params{ .text_string{
				.color = style.get_color_text(),
				.specific{
					.style = ruis::res::font::style::bold
				}
			 } }
		},
		kcal.goal
	);

	auto weight_text = m::text(c,
		{
			.layout_params{
				.align = {ruis::align::front, ruis::align::front}
			}
		},
		loc.get("weight"sv).format({utki::to_utf32(day.get().get_weight_string())})
	);

	// Total calories and the day goal on the first line, weight on the second line
	auto header = m::column(c,
		{
			.layout_params{
				.dims = {ruis::dim::fill, ruis::dim::min}
			}
		},
		{
			// Line 1: "Total: XXX/YYY kcal"
			m::row(c,
				{
					.layout_params{
						.dims = {ruis::dim::min, ruis::dim::min},
						.align = {ruis::align::front, ruis::align::center}
					}
				},
				{
					m::text(c,
						{
							.layout_params{
								.align = {ruis::align::front, ruis::align::center}
							}
						},
						loc.get("total_label"sv)
					),
					// gap separating the "Total:" label from the numbers
					m::gap(c,
						{
							.layout_params{
								.dims = {style.get_len_gap_small(), 0_pp}
							}
						}
					),
					std::move(total_text),
					std::move(goal_text)
				}
			),
			m::gap(c,
				{
					.layout_params{
						.dims = {0_pp, style.get_len_gap_small()}
					}
				}
			),
			// Line 2: "weight: ZZ kg"
			std::move(weight_text)
		}
	);

	auto separator = m::rectangle(c,
		{
			.layout_params{
				.dims = {ruis::dim::fill, style.get_len_gap_small()}
			},
			.params{
				.specific{
					.fill_color = style.get_color_secondary()
				}
			}
		}
	);
	// clang-format on

	// The day's entries.
	// The list requests as much height as all its items need, but the layouts clamp it to the
	// space actually available (see linear_layout::lay_out()), so the dialog never gets bigger
	// than the screen, the list is resized as the available space changes and its items are
	// scrolled when they do not fit.
	// A 'fill' height dimension - with or without a weight - would not work here: the dialog
	// panel is sized to fit its contents, so a fill-sized child contributes nothing to that
	// size and would collapse to zero height.
	utki::shared_ref<ruis::widget> entries_widget = [&]() -> utki::shared_ref<ruis::widget> {
		if (day.get().entries.empty()) {
			// Wrapped in the same padding as the list items
			// clang-format off
			return m::padding(c,
				{
					.layout_params{
						.dims = {ruis::dim::fill, ruis::dim::min
						}
					},
					.params{
						.container{
							.layout = ruis::layout::column
						},
						.specific{
							.borders = {style.get_len_gap()}
						}
					}
            	},
				{
					m::text(c,
					{
						// centered horizontally within the padding
						.layout_params{
							.dims = {ruis::dim::min, ruis::dim::min},
							.align = {ruis::align::center, ruis::align::front}
					 	},
					 	.params{ .text_string{
							.color = style.get_color_text_secondary(),
							.font{
								.size = style.get_font_size_secondary()
							}
						 } }
					},
					loc.get("day_entries_dialog:no_entries"sv)
				)}
			);
			// clang-format on
		}

		// clang-format off
		return m::list(c,
			{
				.layout_params{
					// 'max' in the long direction: the list is measured as 'min' (its natural size is
					// the sum of its items), but it is laid out as 'fill': it gets its share of the
					// space left over by its siblings, so it shrinks when the dialog is too short for
					// its contents and the button row below it stays fully visible
					.dims = {ruis::dim::fill, ruis::dim::max},
					.weight = 1
				},
				.params{
					.specific{
						.provider = utki::make_unique<day_entries_list_provider>(c, day)
					}
				}
			}
		);
		// clang-format on
	}();

	// Create the dialog with its content
	// clang-format off
	auto dialog = ruis::touch::make::dialog(c,
		{
			.layout_params{
				.dims = {ruis::dim::fill, ruis::dim::fill}
			}
		},
		{
			m::text(c,
				{
					.params{ .text_string{
						.font{
							.size = style.get_font_size_title()
						}
					 } }
				},
				title
			),
			make_vert_gap(),
			std::move(header),
			make_vert_gap(),
			std::move(separator),
			std::move(entries_widget),
			make_vert_gap(),
			m::row(c,
				{
					.layout_params{
						.dims = {ruis::dim::fill, ruis::dim::min},
						.align = {ruis::align::front, ruis::align::back}
					}
				},
				{
					std::move(ok_button)
				}
			)
		}
	);
	// clang-format on

	// Show the dialog
	c.get().post_to_ui_thread([olay = utki::make_shared_from(olay), dialog]() {
		olay.get().push_back(dialog);
	});
}

} // namespace calslog

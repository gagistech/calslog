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

#include <cmath>

#include <ruis/widget/button/impl/ellipse_push_button.hpp>
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

#include "food_edit_dialog.hpp"
#include "style.hpp"

using namespace std::string_literals;
using namespace std::string_view_literals;

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
		const auto color_secondary = style.get_color_secondary();
		const auto font_size_secondary = style.get_font_size_secondary();
		const auto color_text_secondary = style.get_color_text_secondary();
		const auto color_text = style.get_color_text();

		// Three dots button on the right side of the item; pressing it opens the
		// food edit dialog for this food.
		// clang-format off
		auto menu_button = m::ellipse_push_button(this->context,
			{
				.layout_params{
					.dims = {ruis::dim::min, ruis::dim::fill},
					.align = {ruis::align::back, ruis::align::center}
				},
				.params{
					.ellipse_button{
						.ellipse{
							.padding{
								.specific{
									.borders = {len_gap_small}
								}
							}
						},
						.specific{
							.unpressed_color = ruis::color::transparent
						}
					}
				}
			},
			{
				m::image(this->context,
					{
						.layout_params{
							.dims = {ruis::dim::min, ruis::dim::fill}
						},
						.params{
							.color = color_text,
							.specific{
								.source = this->context.get().loader().load<ruis::res::image>("ruis_img_more"sv),
								.keep_aspect_ratio = true
							}
						}
					}
				)
			}
		);
		menu_button.get().click_handler = [index](ruis::push_button& b) {
			show_food_edit_dialog(b, index);
		};
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
								// Line 1: food name on the left, kcal/serving on the right
								m::padding(this->context,
									{
										.layout_params{
											.dims = {ruis::dim::fill, ruis::dim::min}
										},
										.params{
											.container{
												.layout = ruis::layout::row
											}
										}
									},
									{
										// Food name, default font, fills the remaining width
										m::text(this->context,
											{
												.layout_params{
													.dims = {ruis::dim::fill, ruis::dim::min},
													.weight = 1
												}
											},
											food.name
										),
										// kcal per serving on the right
										m::text(this->context,
											{
												.layout_params{
													.dims = {ruis::dim::min, ruis::dim::min},
													.align = {ruis::align::back, ruis::align::center}
												}
											},
											// kcal per serving = round(kcal/100g * mass of serving)
											this->context.get().localization.get()
												.get("kcal"sv)
												.format({
													utki::to_utf32(utki::to_string(uint32_t(std::round(food.kcal * food.mass / 100.f))))
												})
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
									// Pass the wording (not a string snapshot) so that the text
									// is re-resolved against the current localization on reload
									// (e.g. after a language change).
									this->context.get().localization.get()
										.get("foods_page:entry_detail"sv)
										.format({
												utki::to_utf32(utki::to_string(food.kcal)),
												utki::to_utf32(utki::to_string(food.mass))
										})
								)
							}
						),
						// Gap before the three dots button
						m::gap(this->context,
							{
								.layout_params{
									.dims = {ruis::dimension(len_gap), ruis::dim::min}
								}
							}
						),
						// Three dots button on the right, vertically centered
						std::move(menu_button)
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
class foods_page :
	public ruis::page, //
	private ruis::container
{
private:
	utki::shared_ref<ruis::rectangle_push_button> fab_button;
	utki::shared_ref<ruis::touch::list> list_widget;

	foods_page(
		const utki::shared_ref<ruis::context>& context, //
		utki::shared_ref<ruis::touch::list> list_widget, //
		utki::shared_ref<ruis::rectangle_push_button> fab_button_param //
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
						list_widget,
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
		list_widget(list_widget)
	// clang-format on
	{}

public:
	foods_page(const utki::shared_ref<ruis::context>& context) :
		// clang-format off
		foods_page(
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
							.provider = utki::make_unique<foods_page_provider>(context)
						}
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
					m::image(
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
		)
	// clang-format on
	{
		// Set click handler on the FAB button: it opens the "add food" dialog.
		this->fab_button.get().click_handler = [](ruis::push_button& b) {
			show_food_edit_dialog(b);
		};

		// Listen to model changes and refresh the foods list, so that edits made
		// through the food edit dialog (and new foods added through the add food
		// dialog) are reflected.
		// No explicit disconnect is required: the model (and its
		// model_changed_signal) is a member of the application and is destroyed
		// before the GUI, so the signal can never emit into a dangling page.
		application::inst().model.model_changed_signal.connect([this]() {
			this->list_widget.get().get_provider().notify_model_change();
		});
	}
};
} // namespace

utki::shared_ref<ruis::page> make_foods_page(const utki::shared_ref<ruis::context>& context)
{
	return utki::make_shared<foods_page>(context);
}

} // namespace calslog
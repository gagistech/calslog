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

#include "log_food_dialog.hpp"

#include <ruis/widget/button/impl/rectangle_push_button.hpp>
#include <ruis/widget/group/overlay.hpp>
#include <ruis/widget/group/touch/dialog.hpp>
#include <ruis/widget/input/labeled_text_field.hpp>
#include <ruis/widget/label/gap.hpp>
#include <ruis/widget/label/text.hpp>
#include <ruis/widget/widget.hpp>

#include "style.hpp"

using namespace std::string_literals;
using namespace std::string_view_literals;

namespace calslog {

void show_log_food_dialog(ruis::widget& parent_widget)
{
    auto& c = parent_widget.context;

    auto& olay = parent_widget.get_ancestor<ruis::overlay>();

    // Create the Add button separately so we can set its click handler
    // clang-format off
    auto add_button = m::rectangle_push_button(c,
        {
            .layout_params{
                .dims = {ruis::dim::fill, ruis::dim::min}
            },
            .params{
                .rectangle_button{
                    .specific{
                        // TODO: add special button?
                        .unpressed_color = c.get().style().get_color_special()
                    }
                }
            }
        },
        {
            m::text(c,
                {
                    .params{
                        .font{
                            .size = c.get().style().get_font_size_normal()
                        }
                    }
                },
                c.get().localization.get().get("log_food_dialog:add_button"sv)
            )
        }
    );
    // clang-format on

    // Helper to create a labeled text field with a localized label
    auto make_field = [&c](std::string_view label_loc_id) {
        return m::labeled_text_field(c,
            {
                .layout_params{
                    .dims = {ruis::dim::fill, ruis::dim::min}
                },
                .params{
                    .label{
                        .string = c.get().localization.get().get(label_loc_id)
                    }
                }
            },
            ruis::string()
        );
    };

    // Helper to create a gap with the standard vertical spacing
    auto make_gap = [&c]() {
        return m::gap(c,
            {
                .layout_params{
                    .dims = {ruis::dim::fill, c.get().style().get_len_gap()}
                }
            }
        );
    };

    // Create the dialog with its content
    // clang-format off
    auto dialog = ruis::touch::make::dialog(c,
        {
            .layout_params{
                .dims = {ruis::dim::fill, ruis::dim::fill}
            },
            .params{
                .layout = ruis::layout::column
            }
        },
        {
            m::text(c,
                {
                    .params{
                        .font{
                            .size = c.get().style().get_font_size_title()
                        }
                    }
                },
                c.get().localization.get().get("log_food_dialog:title"sv)
            ),
            make_gap(),
            make_field("log_food_dialog:food_name"sv),
            make_gap(),
            make_field("log_food_dialog:calories_per_100g"sv),
            make_gap(),
            make_field("log_food_dialog:food_mass"sv),
            std::move(add_button)
        }
    );
    // clang-format on

    // Show the dialog
    c.get().post_to_ui_thread([olay = utki::make_shared_from(olay), dialog]() {
        olay.get().push_back(dialog);
    });
}

} // namespace calslog

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

#include <ruis/widget/button/impl/image_push_button.hpp>
#include <ruis/widget/button/impl/rectangle_push_button.hpp>
#include <ruis/widget/group/overlay.hpp>
#include <ruis/widget/group/touch/dialog.hpp>
#include <ruis/widget/group/touch/list.hpp>
#include <ruis/widget/label/gap.hpp>
#include <ruis/widget/label/image.hpp>
#include <ruis/widget/label/padding.hpp>
#include <ruis/widget/label/text.hpp>
#include <utki/string.hpp>

#include "style.hpp"
#include "../application.hpp"
#include "../model/model.hpp"

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
        const float total_kcal = entry.kcal * entry.mass * entry.pcs / 100.0f;

        // clang-format off
        return m::padding(this->context,
            {
                .layout{
                    .dims = {ruis::dim::fill, ruis::dim::min}
                },
                .params{
                    .container{
                        .layout = ruis::layout::row
                    },
                    .specific{
                        .borders = {ruis::length::make_pp(10)}
                    }
                }
            },
            {
                m::text(this->context,
                    {
                        .params{
                            .font{
                                .size = ruis::length::make_pp(20)
                            }
                        }
                    },
                    entry.name
                ),
                m::gap(this->context,
                    {
                        .layout{
                            .dims = {ruis::dim::fill, ruis::dim::min}
                        }
                    }
                ),
                m::text(this->context,
                    {
                        .params{
                            .color{
                                .normal = 0xff808080
                            },
                            .font{
                                .size = ruis::length::make_pp(20)
                            }
                        }
                    },
                    utki::to_utf32(utki::cat(entry.pcs)) + U" x " + utki::to_utf32(utki::cat(entry.mass)) + U"g = " + utki::to_utf32(utki::cat(total_kcal)) + U" kcal"
                )
            }
        );
        // clang-format on
    }
};
} // namespace

namespace {
void show_add_dialog(ruis::widget& parent_widget)
{
    auto& c = parent_widget.context;

    auto& olay = parent_widget.get_ancestor<ruis::overlay>();

    // Create the Add button separately so we can set its click handler
    // clang-format off
    auto add_button = m::rectangle_push_button(c,
        {
            .layout{
                .dims = {ruis::dim::min, ruis::dim::min}
            },
            .params{
                .rectangle_button{
                    .rectangle{
                        .padding{
                            .specific{
                                .borders = {c.get().style().get_len_button_padding()}
                            }
                        },
                        .specific{
                            .corner_radii = {c.get().style().get_len_button_padding()}
                        }
                    },
                    .specific{
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

    // Create the dialog with its content
    // clang-format off
    auto dialog = ruis::touch::make::dialog(c,
        {
            .layout{
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
            m::gap(c,
                {
                    .layout{
                        .dims = {ruis::dim::fill, c.get().style().get_len_gap()}
                    }
                }
            ),
            m::padding(c,
                {
                    .layout{
                        .dims = {ruis::dim::fill, ruis::dim::min}
                    },
                    .params{
                        .container{
                            .layout = ruis::layout::row
                        },
                        .specific{
                            .borders = {c.get().style().get_len_gap()}
                        }
                    }
                },
                {
                    m::gap(c,
                        {
                            .layout{
                                .dims = {ruis::dim::fill, ruis::dim::min},
                                .weight = 1
                            }
                        }
                    ),
                    std::move(add_button)
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

class today_page :
    public ruis::page, //
    private ruis::container
{
private:
    utki::shared_ref<ruis::rectangle_push_button> fab_button;

    today_page(
        const utki::shared_ref<ruis::context>& context, //
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
                    .layout = ruis::layout::pile
                }
            },
            {
                std::move(list_widget),
                m::padding(
                    context,
                    {
                        .layout{
                            .align = {ruis::align::back, ruis::align::back}
                        },
                        .params{
                            .specific{
                                .borders = {16_pp} // TODO: make multiplier of gap?
                            }
                        }
                    },
                    {
                        fab_button_param
                    }
                )
            }
        ),
        fab_button(fab_button_param)
    // clang-format on
    {}

public:
    today_page(const utki::shared_ref<ruis::context>& context) :
        today_page(
            context,
            // Create the list widget
            // clang-format off
            ruis::touch::make::list(
                context,
                {
                    .layout{
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
                    .layout{
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
                            .layout{
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
            show_add_dialog(b);
        };
    }
};
} // namespace

utki::shared_ref<ruis::page> make_today_page(const utki::shared_ref<ruis::context>& context)
{
    return utki::make_shared<today_page>(context);
}

} // namespace calslog
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

#include "application.hpp"

#include <iomanip>

#include <clargs/parser.hpp>
#include <fsif/native_file.hpp>
#include <ruis/widget/group/overlay.hpp>
#include <utki/debug.hpp>

#include "gui.hpp"

using namespace std::string_literals;
using namespace std::string_view_literals;

using namespace calslog;

namespace {
constexpr auto screen_dims = r4::vector2<unsigned>(1116, 2484) / 3;
} // namespace

application::application(
	bool windowed, //
	std::string_view res_path
) :
	ruisapp::application({
		.name = "calslog"s //
	}),
	res_path(fsif::as_dir(res_path))
{
	auto& win = this->make_window({
		.dims = screen_dims,
		.fullscreen = !windowed
	});

	win.gui.context.get().window().close_handler = [this]() {
		this->quit();
	};

	win.gui.init_standard_widgets(this->get_res_file());

	win.gui.context.get().loader().mount_res_pack(this->get_res_file(this->res_path));

	auto c = make_root_widget(win.gui.context);

	win.gui.set_root(c);
}

std::unique_ptr<application> calslog::make_application(
	std::string_view executable, //
	utki::span<std::string_view> args
)
{
	bool windowed = false;

	std::string res_path = []() {
		fsif::native_file local_share("/usr/local/share/calslog/"sv);

		if (local_share.exists()) {
			return local_share.path();
		}

		return "/usr/share/calslog/"s;
	}();

	clargs::parser p;

	p.add("window", "run in window mode", [&]() {
		windowed = true;
	});

	p.add("res-path", "resources path, default = /usr/share/calslog/", [&](std::string_view v) {
		res_path = v;
	});

	p.parse(args);

	return std::make_unique<application>(windowed, res_path);
}

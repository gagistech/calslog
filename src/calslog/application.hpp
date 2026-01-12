/*
calslog - wall calslog control example GUI project

Copyright (C) 2025 Gagistech Oy <gagistechoy@gmail.com>

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

#pragma once

#include <ruisapp/application.hpp>

namespace calslog {

class application : public ruisapp::application
{
public:
	const std::string res_path;

	application(bool window, std::string_view res_path);

	static application& inst()
	{
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast, "here we are 100% sure the cast is safe")
		return static_cast<application&>(ruisapp::application::inst());
	}
};

std::unique_ptr<application> make_application(
	std::string_view executable, //
	utki::span<std::string_view> args
);

} // namespace calslog

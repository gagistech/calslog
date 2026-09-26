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

#include "settings.hpp"

#include <filesystem>
#include <ranges>

#include <fsif/native_file.hpp>
#include <utki/config.hpp>

using namespace std::string_view_literals;

using namespace calslog;

namespace {
constexpr const auto settings_filename = "settings.tml"sv;

constexpr const auto day_flip_minutes_key = "day_flip_minutes"sv;
constexpr const auto language_key = "language"sv;
} // namespace

namespace {
size_t language_id_to_index(std::string_view id)
{
	const auto& lang_mapping = settings_model::language_id_to_name_mapping;

	auto i = std::ranges::find_if(
		lang_mapping, //
		[&](const auto& a) {
			return a.first == id;
		}
	);

	if (i == lang_mapping.end()) {
		return 0;
	}

	return std::distance(lang_mapping.begin(), i);
}

std::string_view language_index_to_id(size_t index)
{
	const auto& lang_mapping = settings_model::language_id_to_name_mapping;

	utki::assert(index < lang_mapping.size(), SL);

	return lang_mapping[index].first;
}
} // namespace

settings::settings(std::string_view config_dir) :
	filename(utki::cat(config_dir, settings_filename)),
	settings_v(read(this->filename))
{}

settings_model settings::read(std::string_view filename)
{
	fsif::native_file fi(filename);

	if (!fi.exists()) {
		utki::log_debug([](auto& o) {
			o << "settings file not found, use default settings" << std::endl;
		});
		return {};
	}

	settings_model ret;

	auto tml = tml::read(fi);

	for (const auto& t : tml) {
		if (t.value.string == day_flip_minutes_key) {
			if (!t.children.empty()) {
				ret.day_flip_minutes = t.children.front().value.to_uint32();
			}
		} else if (t.value.string == language_key) {
			if (!t.children.empty()) {
				ret.cur_language_index = language_id_to_index(t.children.front().value.string);
			}
		}
	}

	return ret;
}

void settings::set(const settings_model& s)
{
	this->settings_v = s;

#if CFG_OS_NAME != CFG_OS_NAME_EMSCRIPTEN
	this->write();
#endif

	this->settings_changed_signal.emit(this->settings_v);
}

void settings::write()
{
	settings_model default_values;

	tml::forest tml;

	auto add_setting = [&tml](std::string_view key, tml::leaf val) {
		tml.push_back(tml::tree(key, {val}));
	};

	if (this->settings_v.day_flip_minutes != default_values.day_flip_minutes) {
		add_setting(day_flip_minutes_key, tml::leaf(this->settings_v.day_flip_minutes));
	}

	if (this->settings_v.cur_language_index != 0) {
		add_setting(language_key, tml::leaf(language_index_to_id(this->settings_v.cur_language_index)));
	}

	std::filesystem::create_directories(std::filesystem::path(this->filename).parent_path());

	fsif::native_file fi(filename);
	tml::write(tml, fi);
}

constexpr decltype(settings_model::language_id_to_name_mapping) settings_model::language_id_to_name_mapping{
	{
     {"en"sv, U"English"sv},
     {"fi"sv, U"Suomi"sv},
     {"ru"sv, U"Русский"sv},
	}
};

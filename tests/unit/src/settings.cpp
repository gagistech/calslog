#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

#include <calslog/settings.hpp>
#include <calslog/util/day_flip_time.hpp>
#include <tst/check.hpp>
#include <tst/set.hpp>
#include <utki/string.hpp>
#include <utki/unicode.hpp>

using namespace std::string_literals;
using namespace std::string_view_literals;

namespace {

// RAII temporary directory for the settings storage tests.
struct temp_dir {
	std::filesystem::path path;

	explicit temp_dir(std::string name)
	{
		this->path = std::filesystem::temp_directory_path() / ("calslog_test_settings_" + std::move(name));
		std::filesystem::remove_all(this->path);
		std::filesystem::create_directories(this->path);
	}

	~temp_dir()
	{
		std::filesystem::remove_all(this->path);
	}
};

std::string read_file(const std::filesystem::path& p)
{
	std::ifstream fi(p);
	return std::string((std::istreambuf_iterator<char>(fi)), std::istreambuf_iterator<char>());
}

// calslog::settings concatenates the given directory and the file name
// without a path separator, so the directory must end with a slash.
std::string dir_with_slash(const temp_dir& td)
{
	return td.path.string() + "/";
}

const tst::set set("settings", [](tst::suite& suite) {
	suite.add("defaults_when_file_absent", []() {
		temp_dir td("defaults_when_file_absent");

		calslog::settings s(dir_with_slash(td));

		const auto& v = s.get();
		tst::check_eq(v.day_flip_minutes, uint32_t(calslog::settings_model::default_day_flip_minutes));
		tst::check_eq(v.cur_language_index, size_t(0));
		tst::check(v.cur_theme == ruis::theme::dark);
	});

	suite.add("write_then_read_round_trip", []() {
		temp_dir td("write_then_read_round_trip");

		{
			calslog::settings s(dir_with_slash(td));

			auto v = s.get();
			v.day_flip_minutes = 240; // 4:00
			v.cur_language_index = 2; // ru
			v.cur_theme = ruis::theme::light;
			s.set(v);
		}

		calslog::settings s2(dir_with_slash(td));

		tst::check_eq(s2.get().day_flip_minutes, uint32_t(240));
		tst::check_eq(s2.get().cur_language_index, size_t(2));
		tst::check(s2.get().cur_theme == ruis::theme::light);
	});

	suite.add("default_values_not_persisted", []() {
		temp_dir td("default_values_not_persisted");

		{
			calslog::settings s(dir_with_slash(td));

			auto v = s.get();
			v.cur_language_index = 2; // only the language differs from the defaults
			s.set(v);
		}

		auto content = read_file(td.path / "settings.tml");

		// default values are not written to the file
		tst::check(content.find("day_flip_minutes") == std::string::npos);
		tst::check(content.find("theme") == std::string::npos);
		// non-default values are written
		tst::check(content.find("language{ru}") != std::string::npos);
	});

	suite.add("device_file_round_trip", []() {
		// The exact byte sequence that was observed in settings.tml on a real device.
		// The stored value must pass losslessly through the dialog's text representation:
		// file -> model value -> field text -> parse -> model value.
		temp_dir td("device_file_round_trip");

		{
			std::ofstream fi(td.path / "settings.tml");
			fi << "day_flip_minutes{4}\nlanguage{ru}\n";
		}

		calslog::settings s(dir_with_slash(td));

		tst::check_eq(s.get().day_flip_minutes, uint32_t(4));
		tst::check_eq(s.get().cur_language_index, size_t(2)); // ru

		// the text the settings dialog puts into the field for the loaded value
		auto text = calslog::format_day_flip_time(s.get().day_flip_minutes);
		tst::check_eq(text, U"0:04"s);

		// if the user presses Save without editing the field,
		// the saved value must be identical to the loaded one
		tst::check(calslog::is_valid_day_flip_time(utki::to_utf8(text)));
		tst::check_eq(calslog::parse_day_flip_time(utki::to_utf8(text)), uint32_t(4));
	});

	suite.add("user_typed_value_round_trip", []() {
		// Simulates the user typing "4:00" into the field and pressing Save:
		// typing -> field text -> parse -> model value -> file -> read -> field text.
		temp_dir td("user_typed_value_round_trip");

		const std::u32string typed = U"4:00";
		const uint32_t expected = 4 * 60; // 240

		// typing "4:00" into an empty field must be accepted by the filter
		// character by character
		std::u32string text;
		for (auto c : typed) {
			tst::check(calslog::day_flip_time_input_filter(
				text, //
				text.size(),
				text.size(),
				std::u32string_view(&c, 1)
			));
			text += c;
		}
		tst::check_eq(text, typed);

		// parsing the typed text must yield the intended time of day
		tst::check_eq(calslog::parse_day_flip_time(utki::to_utf8(typed)), expected);

		{
			calslog::settings s(dir_with_slash(td));

			auto v = s.get();
			v.day_flip_minutes = calslog::parse_day_flip_time(utki::to_utf8(typed));
			s.set(v);
		}

		// saving and re-reading must preserve the value, and the field must
		// display the value the user typed
		calslog::settings s2(dir_with_slash(td));

		tst::check_eq(s2.get().day_flip_minutes, expected);
		tst::check_eq(calslog::format_day_flip_time(s2.get().day_flip_minutes), typed);
	});

	suite.add("changed_signal", []() {
		temp_dir td("changed_signal");

		calslog::settings s(dir_with_slash(td));

		bool fired = false;
		uint32_t got = 0;
		auto conn = s.settings_changed_signal.connect([&](const calslog::settings_model& m) {
			fired = true;
			got = m.day_flip_minutes;
		});

		auto v = s.get();
		v.day_flip_minutes = 5 * 60; // 5:00
		s.set(v);

		tst::check(fired);
		tst::check_eq(got, uint32_t(300));

		s.settings_changed_signal.disconnect(conn);
	});
});
} // namespace

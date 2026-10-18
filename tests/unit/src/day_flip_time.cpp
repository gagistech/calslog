#include <cstddef>
#include <optional>
#include <string_view>

#include <calslog/util/day_flip_time.hpp>
#include <tst/check.hpp>
#include <tst/set.hpp>
#include <utki/unicode.hpp>

using namespace std::string_literals;

namespace {
const tst::set set("day_flip_time", [](tst::suite& suite) {
	suite.add("format", []() {
		tst::check_eq(calslog::format_day_flip_time(0), U"0:00"s); // 00:00
		tst::check_eq(calslog::format_day_flip_time(3), U"0:03"s); // 00:03
		tst::check_eq(calslog::format_day_flip_time(59), U"0:59"s); // 00:59
		tst::check_eq(calslog::format_day_flip_time(180), U"3:00"s); // 03:00
		tst::check_eq(calslog::format_day_flip_time(240), U"4:00"s); // 04:00
		tst::check_eq(calslog::format_day_flip_time(750), U"12:30"s); // 12:30
		tst::check_eq(calslog::format_day_flip_time(1439), U"23:59"s); // 23:59
	});

	suite.add("parse_valid", []() {
		tst::check_eq(calslog::parse_day_flip_time("0:00"), uint32_t(0));
		tst::check_eq(calslog::parse_day_flip_time("0:03"), uint32_t(3));
		tst::check_eq(calslog::parse_day_flip_time("03:00"), uint32_t(180)); // 2-digit hours
		tst::check_eq(calslog::parse_day_flip_time("3:00"), uint32_t(180));
		tst::check_eq(calslog::parse_day_flip_time("4:00"), uint32_t(240));
		tst::check_eq(calslog::parse_day_flip_time("12:30"), uint32_t(750));
		tst::check_eq(calslog::parse_day_flip_time("23:59"), uint32_t(1439));
		tst::check_eq(calslog::parse_day_flip_time("0.03"), uint32_t(3)); // '.' separator
		tst::check_eq(calslog::parse_day_flip_time("3.00"), uint32_t(180));
		tst::check_eq(calslog::parse_day_flip_time(" 3:00"), uint32_t(180)); // surrounding whitespace tolerated
		tst::check_eq(calslog::parse_day_flip_time("3:00 "), uint32_t(180));
	});

	suite.add("parse_invalid", []() {
		auto parse_throws = [](std::string_view str) {
			bool threw = false;
			try {
				calslog::parse_day_flip_time(str);
			} catch (const std::invalid_argument&) {
				threw = true;
			}
			return threw;
		};

		tst::check(parse_throws("")); // empty
		tst::check(parse_throws("3")); // no separator
		tst::check(parse_throws("3:")); // no minutes
		tst::check(parse_throws(":00")); // no hours
		tst::check(parse_throws("3:0")); // 1-digit minutes
		tst::check(parse_throws("3.0")); // 1-digit minutes
		tst::check(parse_throws("3:000")); // 3-digit minutes
		tst::check(parse_throws("123:00")); // 3-digit hours
		tst::check(parse_throws("24:00")); // hours out of range
		tst::check(parse_throws("24:0")); // hours out of range
		tst::check(parse_throws("3:60")); // minutes out of range
		tst::check(parse_throws("3:00:00")); // trailing garbage
		tst::check(parse_throws("3-00")); // wrong separator
		tst::check(parse_throws("abc")); // not a time at all
		tst::check(parse_throws("3 :00")); // inner whitespace
		tst::check(parse_throws("0x3:00")); // hex is not accepted
	});

	suite.add("format_parse_round_trip", []() {
		// The crucial invariant: any value that can be stored in the settings model,
		// once displayed in the settings dialog field, must parse back to itself
		// when the user saves the field without editing it.
		for (uint32_t v = 0; v < 1440; ++v) {
			auto text = calslog::format_day_flip_time(v);
			tst::check_eq(calslog::parse_day_flip_time(utki::to_utf8(text)), v);
		}
	});

	suite.add("prefix", []() {
		// valid prefixes (can be extended to a valid time string)
		tst::check(calslog::is_day_flip_time_prefix(U"")); // empty field
		tst::check(calslog::is_day_flip_time_prefix(U"0"));
		tst::check(calslog::is_day_flip_time_prefix(U"3"));
		tst::check(calslog::is_day_flip_time_prefix(U"04"));
		tst::check(calslog::is_day_flip_time_prefix(U"23"));
		tst::check(calslog::is_day_flip_time_prefix(U"3:"));
		tst::check(calslog::is_day_flip_time_prefix(U"3:0"));
		tst::check(calslog::is_day_flip_time_prefix(U"0:0"));
		tst::check(calslog::is_day_flip_time_prefix(U"0:03"));
		tst::check(calslog::is_day_flip_time_prefix(U"3.00"));
		tst::check(calslog::is_day_flip_time_prefix(U"23:5"));
		tst::check(calslog::is_day_flip_time_prefix(U"23:59"));

		// invalid: cannot be extended to a valid time string
		tst::check(!calslog::is_day_flip_time_prefix(U"24")); // hours out of range
		tst::check(!calslog::is_day_flip_time_prefix(U"123")); // 3-digit hours
		tst::check(!calslog::is_day_flip_time_prefix(U":00")); // separator first
		tst::check(!calslog::is_day_flip_time_prefix(U"3::")); // second separator
		tst::check(!calslog::is_day_flip_time_prefix(U"3:6")); // minutes prefix out of range
		tst::check(!calslog::is_day_flip_time_prefix(U"5:6"));
		tst::check(!calslog::is_day_flip_time_prefix(U"3:000")); // 3-digit minutes
		tst::check(!calslog::is_day_flip_time_prefix(U"3:60")); // minutes out of range
		tst::check(!calslog::is_day_flip_time_prefix(U"3-00")); // wrong separator
		tst::check(!calslog::is_day_flip_time_prefix(U"a")); // not a digit
	});

	suite.add("input_filter", []() {
		// Applies the edit the way the text input widget does and returns the new
		// text, or std::nullopt if the filter rejects the edit.
		auto apply = [](std::u32string_view orig, size_t start, size_t end, std::u32string_view ins
					 ) -> std::optional<std::u32string> {
			if (!calslog::day_flip_time_input_filter(orig, start, end, ins)) {
				return std::nullopt;
			}
			auto r = std::u32string(orig.substr(0, start));
			r.append(ins);
			r.append(orig.substr(end));
			return r;
		};

		auto insert_at_end = [&apply](std::u32string_view orig, char32_t c) -> std::optional<std::u32string> {
			return apply(orig, orig.size(), orig.size(), std::u32string_view(&c, 1));
		};

		// typing "4:00" into an empty field, character by character
		tst::check_eq(*insert_at_end(U"", '4'), U"4"s);
		tst::check_eq(*insert_at_end(U"4", ':'), U"4:"s);
		tst::check_eq(*insert_at_end(U"4:", '0'), U"4:0"s);
		tst::check_eq(*insert_at_end(U"4:0", '0'), U"4:00"s);
		tst::check_eq(calslog::parse_day_flip_time(utki::to_utf8(U"4:00")), uint32_t(240));

		// overtyping is rejected
		tst::check(!insert_at_end(U"4:00", '0').has_value()); // "4:000"
		tst::check(!insert_at_end(U"4:", '6').has_value()); // "4:6"
		tst::check(!insert_at_end(U"", 'a').has_value()); // non-digit
		tst::check(!insert_at_end(U"", ':').has_value()); // separator first
		tst::check(!calslog::day_flip_time_input_filter(U"", 0, 0, U"24")); // "24" hours out of range

		// deleting is always fine (any prefix is valid)
		tst::check_eq(*apply(U"4:00", 3, 4, U""), U"4:0"s); // backspace
		tst::check_eq(*apply(U"4:0", 0, 3, U""), U""s); // delete all

		// replacing the whole text with another valid time
		tst::check_eq(*apply(U"4:00", 0, 4, U"3:00"), U"3:00"s);

		// changing the separator
		tst::check_eq(*apply(U"4:00", 1, 2, U"."), U"4.00"s);

		// inserting in the middle
		tst::check(!apply(U"4:00", 2, 2, U"9").has_value()); // "4:900" rejected
		tst::check_eq(*apply(U"40", 1, 1, U":"), U"4:0"s); // inserting the separator in the middle
	});
});
} // namespace

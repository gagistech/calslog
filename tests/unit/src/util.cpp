#include <chrono>

#include <calslog/util.hpp>
#include <tst/check.hpp>
#include <tst/set.hpp>

namespace {
using namespace std::chrono;

auto ymd(int y, int m, int d)
{
	return year_month_day{year{y}, month{static_cast<unsigned>(m)}, day{static_cast<unsigned>(d)}};
}

const tst::set set("log_date", [](tst::suite& suite) {
	suite.add("flip_first_half", []() {
		const auto date = ymd(2026, 10, 5);
		const uint32_t flip = 180; // 03:00

		// before the flip -> previous day
		tst::check_eq(calslog::log_date_for(date, 0, flip), ymd(2026, 10, 4)); // 00:00
		tst::check_eq(calslog::log_date_for(date, 120, flip), ymd(2026, 10, 4)); // 02:00
		tst::check_eq(calslog::log_date_for(date, 179, flip), ymd(2026, 10, 4)); // 02:59

		// at/after the flip -> current day
		tst::check_eq(calslog::log_date_for(date, 180, flip), ymd(2026, 10, 5)); // 03:00
		tst::check_eq(calslog::log_date_for(date, 240, flip), ymd(2026, 10, 5)); // 04:00
		tst::check_eq(calslog::log_date_for(date, 1439, flip), ymd(2026, 10, 5)); // 23:59
	});

	suite.add("flip_second_half", []() {
		const auto date = ymd(2026, 10, 5);
		const uint32_t flip = 1080; // 18:00

		// before the flip -> current day
		tst::check_eq(calslog::log_date_for(date, 0, flip), ymd(2026, 10, 5)); // 00:00
		tst::check_eq(calslog::log_date_for(date, 720, flip), ymd(2026, 10, 5)); // 12:00
		tst::check_eq(calslog::log_date_for(date, 1079, flip), ymd(2026, 10, 5)); // 17:59

		// at/after the flip -> next day
		tst::check_eq(calslog::log_date_for(date, 1080, flip), ymd(2026, 10, 6)); // 18:00
		tst::check_eq(calslog::log_date_for(date, 1439, flip), ymd(2026, 10, 6)); // 23:59
	});

	suite.add("flip_at_noon_belongs_to_second_half", []() {
		const auto date = ymd(2026, 10, 5);
		const uint32_t flip = 720; // 12:00

		tst::check_eq(calslog::log_date_for(date, 719, flip), ymd(2026, 10, 5)); // 11:59 -> current
		tst::check_eq(calslog::log_date_for(date, 720, flip), ymd(2026, 10, 6)); // 12:00 -> next
	});

	suite.add("flip_at_midnight", []() {
		const auto date = ymd(2026, 10, 5);
		const uint32_t flip = 0; // 00:00

		// the log day coincides with the calendar day
		tst::check_eq(calslog::log_date_for(date, 0, flip), ymd(2026, 10, 5)); // 00:00
		tst::check_eq(calslog::log_date_for(date, 720, flip), ymd(2026, 10, 5)); // 12:00
		tst::check_eq(calslog::log_date_for(date, 1439, flip), ymd(2026, 10, 5)); // 23:59
	});

	suite.add("flip_just_before_midnight", []() {
		const auto date = ymd(2026, 10, 5);
		const uint32_t flip = 1439; // 23:59

		tst::check_eq(calslog::log_date_for(date, 1438, flip), ymd(2026, 10, 5)); // 23:58 -> current
		tst::check_eq(calslog::log_date_for(date, 1439, flip), ymd(2026, 10, 6)); // 23:59 -> next
	});

	suite.add("month_boundary", []() {
		const auto date = ymd(2026, 10, 1);
		const uint32_t flip = 180; // 03:00

		// before the flip on the 1st -> last day of the previous month
		tst::check_eq(calslog::log_date_for(date, 0, flip), ymd(2026, 9, 30));
		// after the flip -> the 1st
		tst::check_eq(calslog::log_date_for(date, 180, flip), ymd(2026, 10, 1));
	});

	suite.add("year_boundary", []() {
		const auto date = ymd(2026, 1, 1);
		const uint32_t flip = 180; // 03:00

		// before the flip on Jan 1 -> Dec 31 of the previous year
		tst::check_eq(calslog::log_date_for(date, 0, flip), ymd(2025, 12, 31));
	});

	suite.add("next_day_across_month_end", []() {
		const auto date = ymd(2026, 10, 31);
		const uint32_t flip = 1080; // 18:00

		// after the flip on the last day of the month -> the 1st of the next month
		tst::check_eq(calslog::log_date_for(date, 1080, flip), ymd(2026, 11, 1));
	});
});
} // namespace

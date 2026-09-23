#include <calslog/model/model.hpp>
#include <fsif/span_file.hpp>
#include <fsif/vector_file.hpp>
#include <tst/check.hpp>
#include <tst/set.hpp>

using namespace std::string_view_literals;
using namespace std::string_literals;

namespace {
const tst::set set("calslog", [](tst::suite& suite) {
	suite.add("read_basic", []() {
		auto tml_str = R"qwertyuiop(
            foods{
                "small egg"{
                    kcal{145}
                    mass{50}
                }
                "medium egg"{
                    kcal{145}
                    mass{60}
                }
                "big egg"{
                    kcal{145}
                    mass{70}
                }
            }

            history{
                2026-08-24{
                    egg{
                        kcal{145}
                        mass{55}
                        pcs{2}
                    }
                    "small egg"{
                        kcal{145}
                        mass{50}
                        pcs{13}
                    }
                }
            }
        )qwertyuiop"sv;

		auto root = calslog::model::read(fsif::span_file(tml_str));

		// === foods ===
		tst::check_eq(root.foods.size(), size_t(3));

		tst::check_eq(root.foods.at(0).kcal, uint32_t(145));
		tst::check_eq(root.foods.at(0).mass, uint32_t(50));
		tst::check_eq(root.foods.at(1).kcal, uint32_t(145));
		tst::check_eq(root.foods.at(1).mass, uint32_t(60));
		tst::check_eq(root.foods.at(2).kcal, uint32_t(145));
		tst::check_eq(root.foods.at(2).mass, uint32_t(70));

		// === history ===
		tst::check_eq(root.history.size(), size_t(1));

		tst::check_eq(
			root.history.at(0).date, //
			std::chrono::year_month_day{std::chrono::year{2026}, std::chrono::month{8}, std::chrono::day{24}}
		);
		tst::check_eq(root.history.at(0).entries.size(), size_t(2));

		tst::check_eq(root.history.at(0).entries.at(0).name, U"egg"s);
		tst::check_eq(root.history.at(0).entries.at(0).kcal, uint32_t(145));
		tst::check_eq(root.history.at(0).entries.at(0).mass, uint32_t(55));
		tst::check_eq(root.history.at(0).entries.at(0).pcs, uint32_t(2));

		tst::check_eq(root.history.at(0).entries.at(1).name, U"small egg"s);
		tst::check_eq(root.history.at(0).entries.at(1).kcal, uint32_t(145));
		tst::check_eq(root.history.at(0).entries.at(1).mass, uint32_t(50));
		tst::check_eq(root.history.at(0).entries.at(1).pcs, uint32_t(13));

		// 'enabled' field is absent from the tml, so it defaults to true
		tst::check_eq(root.history.at(0).entries.at(0).enabled, true);
		tst::check_eq(root.history.at(0).entries.at(1).enabled, true);
	});

	suite.add("write_basic", []() {
		calslog::model::root root;

		// === foods ===
		root.foods.push_back({.name = U"small egg", .kcal = 145, .mass = 50});
		root.foods.push_back({.name = U"medium egg", .kcal = 145, .mass = 60});
		root.foods.push_back({.name = U"big egg", .kcal = 145, .mass = 70});

		// === history ===
		calslog::model::day day;
		day.date = std::chrono::year_month_day{std::chrono::year{2026}, std::chrono::month{8}, std::chrono::day{24}};
		day.entries.push_back({.name = U"egg", .pcs = 2, .mass = 55, .kcal = 145});
		day.entries.push_back({.name = U"small egg", .pcs = 13, .mass = 50, .kcal = 145});
		root.history.push_back(day);

		fsif::vector_file fi;

		calslog::model::write(root, fi);

		auto data = fi.reset_data();

		const std::string expected =
			"foods{" //
			"\n\t\"small egg\"{" //
			"\n\t\tkcal{145}" //
			"\n\t\tmass{50}" //
			"\n\t}" //
			"\n\t\"medium egg\"{" //
			"\n\t\tkcal{145}" //
			"\n\t\tmass{60}" //
			"\n\t}" //
			"\n\t\"big egg\"{" //
			"\n\t\tkcal{145}" //
			"\n\t\tmass{70}" //
			"\n\t}" //
			"\n}" //
			"\n" //
			"history{" //
			"\n\t2026-08-24{" //
			"\n\t\tegg{" //
			"\n\t\t\tkcal{145}" //
			"\n\t\t\tpcs{2}" //
			"\n\t\t\tmass{55}" //
			"\n\t\t\tenabled{true}" //
			"\n\t\t}" //
			"\n\t\t\"small egg\"{" //
			"\n\t\t\tkcal{145}" //
			"\n\t\t\tpcs{13}" //
			"\n\t\t\tmass{50}" //
			"\n\t\t\tenabled{true}" //
			"\n\t\t}" //
			"\n\t\tweight{0}" //
			"\n\t}" //
			"\n}" //
			"\n";

		const std::string str(reinterpret_cast<const char*>(data.data()), data.size());
		tst::check_eq(str, expected);

		// === round trip ===
		auto root2 = calslog::model::read(fsif::span_file(utki::make_span(data)));

		tst::check_eq(root2.foods.size(), size_t(3));

		tst::check_eq(root2.foods.at(0).name, U"small egg"s);
		tst::check_eq(root2.foods.at(0).kcal, uint32_t(145));
		tst::check_eq(root2.foods.at(0).mass, uint32_t(50));
		tst::check_eq(root2.foods.at(1).name, U"medium egg"s);
		tst::check_eq(root2.foods.at(1).kcal, uint32_t(145));
		tst::check_eq(root2.foods.at(1).mass, uint32_t(60));
		tst::check_eq(root2.foods.at(2).name, U"big egg"s);
		tst::check_eq(root2.foods.at(2).kcal, uint32_t(145));
		tst::check_eq(root2.foods.at(2).mass, uint32_t(70));

		tst::check_eq(root2.history.size(), size_t(1));

		tst::check_eq(
			root2.history.at(0).date, //
			std::chrono::year_month_day{std::chrono::year{2026}, std::chrono::month{8}, std::chrono::day{24}}
		);
		tst::check_eq(root2.history.at(0).entries.size(), size_t(2));

		tst::check_eq(root2.history.at(0).entries.at(0).name, U"egg"s);
		tst::check_eq(root2.history.at(0).entries.at(0).kcal, uint32_t(145));
		tst::check_eq(root2.history.at(0).entries.at(0).mass, uint32_t(55));
		tst::check_eq(root2.history.at(0).entries.at(0).pcs, uint32_t(2));

		tst::check_eq(root2.history.at(0).entries.at(1).name, U"small egg"s);
		tst::check_eq(root2.history.at(0).entries.at(1).kcal, uint32_t(145));
		tst::check_eq(root2.history.at(0).entries.at(1).mass, uint32_t(50));
		tst::check_eq(root2.history.at(0).entries.at(1).pcs, uint32_t(13));
	});

	suite.add("write_empty", []() {
		calslog::model::root root;

		fsif::vector_file fi;

		calslog::model::write(root, fi);

		auto data = fi.reset_data();

		// round trip of empty model
		auto root2 = calslog::model::read(fsif::span_file(utki::make_span(data)));

		tst::check_eq(root2.foods.size(), size_t(0));
		tst::check_eq(root2.history.size(), size_t(0));
	});

	suite.add("enabled_flag", []() {
		// === reading the 'enabled' field ===
		auto tml_str = R"qwertyuiop(
			history{
				2026-08-24{
					"disabled egg"{
						kcal{145}
						mass{55}
						pcs{2}
						enabled{false}
					}
					"default egg"{
						kcal{100}
						mass{10}
						pcs{1}
					}
					"explicit egg"{
						kcal{100}
						mass{10}
						pcs{3}
						enabled{true}
					}
				}
			}
		)qwertyuiop"sv;

		auto root = calslog::model::read(fsif::span_file(tml_str));

		tst::check_eq(root.history.size(), size_t(1));
		tst::check_eq(root.history.at(0).entries.size(), size_t(3));

		tst::check_eq(root.history.at(0).entries.at(0).enabled, false);
		tst::check_eq(root.history.at(0).entries.at(1).enabled, true); // absent -> default true
		tst::check_eq(root.history.at(0).entries.at(2).enabled, true);

		// only enabled entries are counted in the day total:
		//   disabled egg:  145*55*2/100 = 159  (disabled, not counted)
		//   default egg:   100*10*1/100 = 10
		//   explicit egg:  100*10*3/100 = 30
		//   total = 10 + 30 = 40
		tst::check_eq(root.history.at(0).calc_total_kcal(), uint32_t(40));

		// === writing the 'enabled' field ===
		calslog::model::root root2;

		root2.foods.push_back({.name = U"small egg", .kcal = 145, .mass = 50});

		calslog::model::day day;
		day.date = std::chrono::year_month_day{std::chrono::year{2026}, std::chrono::month{8}, std::chrono::day{24}};
		day.entries.push_back({.name = U"egg", .pcs = 2, .mass = 55, .kcal = 145, .enabled = false});
		day.entries.push_back({.name = U"small egg", .pcs = 13, .mass = 50, .kcal = 145}); // default enabled
		root2.history.push_back(day);

		fsif::vector_file fi;

		calslog::model::write(root2, fi);

		auto data = fi.reset_data();

		const std::string expected =
			"foods{" //
			"\n\t\"small egg\"{" //
			"\n\t\tkcal{145}" //
			"\n\t\tmass{50}" //
			"\n\t}" //
			"\n}" //
			"\n" //
			"history{" //
			"\n\t2026-08-24{" //
			"\n\t\tegg{" //
			"\n\t\t\tkcal{145}" //
			"\n\t\t\tpcs{2}" //
			"\n\t\t\tmass{55}" //
			"\n\t\t\tenabled{false}" //
			"\n\t\t}" //
			"\n\t\t\"small egg\"{" //
			"\n\t\t\tkcal{145}" //
			"\n\t\t\tpcs{13}" //
			"\n\t\t\tmass{50}" //
			"\n\t\t\tenabled{true}" //
			"\n\t\t}" //
			"\n\t\tweight{0}" //
			"\n\t}" //
			"\n}" //
			"\n";

		const std::string str(reinterpret_cast<const char*>(data.data()), data.size());
		tst::check_eq(str, expected);

		// === round trip preserves the enabled flag ===
		auto root3 = calslog::model::read(fsif::span_file(utki::make_span(data)));

		tst::check_eq(root3.history.size(), size_t(1));
		tst::check_eq(root3.history.at(0).entries.size(), size_t(2));
		tst::check_eq(root3.history.at(0).entries.at(0).enabled, false);
		tst::check_eq(root3.history.at(0).entries.at(1).enabled, true);

		// only the enabled "small egg" is counted: 145*50*13/100 = 942
		tst::check_eq(root3.history.at(0).calc_total_kcal(), uint32_t(942));
	});

	suite.add("weight_field", []() {
		auto weight_str = [](uint32_t w) -> std::string {
			calslog::model::day d;
			d.weight = w;
			return d.get_weight_string();
		};

		// === get_weight_string: grams -> kg string, up to 1 decimal digit ===
		tst::check_eq(weight_str(0), "?"s); // not logged
		tst::check_eq(weight_str(1), "0.0"s); // 0.001 kg
		tst::check_eq(weight_str(100), "0.1"s); // 0.1 kg
		tst::check_eq(weight_str(1234), "1.2"s); // 1.234 kg
		tst::check_eq(weight_str(1250), "1.3"s); // 1.25 kg -> 1.3 (round half up)
		tst::check_eq(weight_str(71500), "71.5"s);
		tst::check_eq(weight_str(72400), "72.4"s);

		// === reading the 'weight' field ===
		auto tml_str = R"qwertyuiop(
			history{
				2026-08-24{
					egg{
						kcal{145}
						mass{55}
						pcs{2}
					}
					weight{72400}
				}
				2026-08-25{
					egg{
						kcal{145}
						mass{55}
						pcs{2}
					}
				}
			}
		)qwertyuiop"sv;

		auto root = calslog::model::read(fsif::span_file(tml_str));

		tst::check_eq(root.history.size(), size_t(2));

		tst::check_eq(root.history.at(0).weight, uint32_t(72400));
		// absent 'weight' field defaults to 0 (not logged)
		tst::check_eq(root.history.at(1).weight, uint32_t(0));

		// the 'weight' node is not treated as a food entry
		tst::check_eq(root.history.at(0).entries.size(), size_t(1));
		tst::check_eq(root.history.at(0).entries.at(0).name, U"egg"s);

		// === writing the 'weight' field ===
		calslog::model::root root2;

		root2.foods.push_back({.name = U"small egg", .kcal = 145, .mass = 50});

		calslog::model::day day;
		day.date = std::chrono::year_month_day{std::chrono::year{2026}, std::chrono::month{8}, std::chrono::day{24}};
		day.entries.push_back({.name = U"egg", .pcs = 2, .mass = 55, .kcal = 145});
		day.weight = 72400;
		root2.history.push_back(day);

		fsif::vector_file fi;

		calslog::model::write(root2, fi);

		auto data = fi.reset_data();

		const std::string expected =
			"foods{" //
			"\n\t\"small egg\"{" //
			"\n\t\tkcal{145}" //
			"\n\t\tmass{50}" //
			"\n\t}" //
			"\n}" //
			"\n" //
			"history{" //
			"\n\t2026-08-24{" //
			"\n\t\tegg{" //
			"\n\t\t\tkcal{145}" //
			"\n\t\t\tpcs{2}" //
			"\n\t\t\tmass{55}" //
			"\n\t\t\tenabled{true}" //
			"\n\t\t}" //
			"\n\t\tweight{72400}" //
			"\n\t}" //
			"\n}" //
			"\n";

		const std::string str(reinterpret_cast<const char*>(data.data()), data.size());
		tst::check_eq(str, expected);

		// === round trip preserves the weight ===
		auto root3 = calslog::model::read(fsif::span_file(utki::make_span(data)));

		tst::check_eq(root3.history.size(), size_t(1));
		tst::check_eq(root3.history.at(0).weight, uint32_t(72400));
		tst::check_eq(root3.history.at(0).entries.size(), size_t(1));
	});
});
} // namespace

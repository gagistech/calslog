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
                    e{
                        name{egg}
                        kcal{145}
                        mass{55}
                        pcs{2}
                    }
                    e{
                        name{"small egg"}
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

		tst::check_eq(root.foods.at(0).kcal, 145.0f);
		tst::check_eq(root.foods.at(0).mass, 50.0f);
		tst::check_eq(root.foods.at(1).kcal, 145.0f);
		tst::check_eq(root.foods.at(1).mass, 60.0f);
		tst::check_eq(root.foods.at(2).kcal, 145.0f);
		tst::check_eq(root.foods.at(2).mass, 70.0f);

		// === history ===
		tst::check_eq(root.history.size(), size_t(1));

		tst::check_eq(
			root.history.at(0).date, //
			std::chrono::year_month_day{std::chrono::year{2026}, std::chrono::month{8}, std::chrono::day{24}}
		);
		tst::check_eq(root.history.at(0).entries.size(), size_t(2));

		tst::check_eq(root.history.at(0).entries.at(0).name, U"egg"s);
		tst::check_eq(root.history.at(0).entries.at(0).kcal, 145.0f);
		tst::check_eq(root.history.at(0).entries.at(0).mass, 55.0f);
		tst::check_eq(root.history.at(0).entries.at(0).pcs, uint32_t(2));

		tst::check_eq(root.history.at(0).entries.at(1).name, U"small egg"s);
		tst::check_eq(root.history.at(0).entries.at(1).kcal, 145.0f);
		tst::check_eq(root.history.at(0).entries.at(1).mass, 50.0f);
		tst::check_eq(root.history.at(0).entries.at(1).pcs, uint32_t(13));
	});

	suite.add("write_basic", []() {
		calslog::model::root root;

		// === foods ===
		root.foods.push_back({.name = U"small egg", .kcal = 145.0f, .mass = 50.0f});
		root.foods.push_back({.name = U"medium egg", .kcal = 145.0f, .mass = 60.0f});
		root.foods.push_back({.name = U"big egg", .kcal = 145.0f, .mass = 70.0f});

		// === history ===
		calslog::model::day day;
		day.date = std::chrono::year_month_day{std::chrono::year{2026}, std::chrono::month{8}, std::chrono::day{24}};
		day.entries.push_back({.name = U"egg", .pcs = 2, .mass = 55.0f, .kcal = 145.0f});
		day.entries.push_back({.name = U"small egg", .pcs = 13, .mass = 50.0f, .kcal = 145.0f});
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
			"\n\t\te{" //
			"\n\t\t\tname{egg}" //
			"\n\t\t\tkcal{145}" //
			"\n\t\t\tpcs{2}" //
			"\n\t\t\tmass{55}" //
			"\n\t\t}" //
			"\n\t\te{" //
			"\n\t\t\tname{\"small egg\"}" //
			"\n\t\t\tkcal{145}" //
			"\n\t\t\tpcs{13}" //
			"\n\t\t\tmass{50}" //
			"\n\t\t}" //
			"\n\t}" //
			"\n}" //
			"\n";

		const std::string str(reinterpret_cast<const char*>(data.data()), data.size());
		tst::check_eq(str, expected);

		// === round trip ===
		auto root2 = calslog::model::read(fsif::span_file(utki::make_span(data)));

		tst::check_eq(root2.foods.size(), size_t(3));

		tst::check_eq(root2.foods.at(0).name, U"small egg"s);
		tst::check_eq(root2.foods.at(0).kcal, 145.0f);
		tst::check_eq(root2.foods.at(0).mass, 50.0f);
		tst::check_eq(root2.foods.at(1).name, U"medium egg"s);
		tst::check_eq(root2.foods.at(1).kcal, 145.0f);
		tst::check_eq(root2.foods.at(1).mass, 60.0f);
		tst::check_eq(root2.foods.at(2).name, U"big egg"s);
		tst::check_eq(root2.foods.at(2).kcal, 145.0f);
		tst::check_eq(root2.foods.at(2).mass, 70.0f);

		tst::check_eq(root2.history.size(), size_t(1));

		tst::check_eq(
			root2.history.at(0).date, //
			std::chrono::year_month_day{std::chrono::year{2026}, std::chrono::month{8}, std::chrono::day{24}}
		);
		tst::check_eq(root2.history.at(0).entries.size(), size_t(2));

		tst::check_eq(root2.history.at(0).entries.at(0).name, U"egg"s);
		tst::check_eq(root2.history.at(0).entries.at(0).kcal, 145.0f);
		tst::check_eq(root2.history.at(0).entries.at(0).mass, 55.0f);
		tst::check_eq(root2.history.at(0).entries.at(0).pcs, uint32_t(2));

		tst::check_eq(root2.history.at(0).entries.at(1).name, U"small egg"s);
		tst::check_eq(root2.history.at(0).entries.at(1).kcal, 145.0f);
		tst::check_eq(root2.history.at(0).entries.at(1).mass, 50.0f);
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
});
} // namespace

#include <calslog/model/model.hpp>
#include <fsif/span_file.hpp>
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
});
} // namespace

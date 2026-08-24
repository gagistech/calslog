#include <tst/set.hpp>
#include <tst/check.hpp>

#include <fsif/span_file.hpp>

#include <calslog/model/model.hpp>

using namespace std::string_view_literals;

namespace{
const tst::set set("calslog", [](tst::suite& suite){
    suite.add("read_basic", [](){
        auto tml_str = R"qwertyuiop(
            foods{
                "small egg"{
                    kcals{145}
                    mass{50}
                }
                "medium egg"{
                    kcals{145}
                    mass{60}
                }
                "big egg"{
                    kcals{145}
                    mass{70}
                }
            }

            history{
                2026-08-24{
                    e{
                        name{egg}
                        kcal{145}
                        mass{50}
                        pcs{2}
                    }
                }
            }
        )qwertyuiop"sv;

        auto root = calslog::model::read(fsif::span_file(tml_str));

        tst::check_eq(root.foods.size(), size_t(3));
        // TODO:
    });
});
}

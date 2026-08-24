#include "model.hpp"

#include <tml/tree.hpp>
#include <utki/string.hpp>
#include <utki/unicode.hpp>

using namespace std::string_view_literals;

using namespace calslog;

/*
=========================
==== tml file format ====

foods{
    <name>{
        kcal{<kcal-per-100-grams>}
        mass{<grams-per-portion>}
    }
}

history{
    <YYYY-MM-DD>{
        e{
            name{<food-name>}
            kcal{<kcal-per-100-g>}
            pcs{<number-of-portions>}
            mass{<grams-per-portion>}
        }
    }
    ...
}

*/

namespace {
constexpr auto kcal_word = "kcal"sv;
constexpr auto mass_word = "mass"sv;
constexpr auto name_word = "name"sv;
constexpr auto pcs_word = "pcs"sv;
} // namespace

namespace {
model::food parse_food(const tml::tree& tree)
{
	float kcal = 0;
	float mass = 0;
	for (auto& c : tree.children) {
		if (c.value == kcal_word) {
			kcal = c.children.at(0).value.to_float();
		} else if (c.value == mass_word) {
			mass = c.children.at(0).value.to_float();
		}
	}
	return {.name = utki::to_utf32(tree.value.string), .kcal = kcal, .mass = mass};
}
} // namespace

namespace {
std::vector<model::food> parse_foods(const tml::forest& forest)
{
	std::vector<model::food> foods;

	for (auto& tree : forest) {
		foods.push_back(parse_food(tree));
	}

	return foods;
}
} // namespace

namespace {
// TODO: move to utki?
std::chrono::year_month_day parse_yyyy_mm_dd(std::string_view str)
{
	utki::string_parser p(str);

	auto y = p.read_number<int>();
	p.skip_char('-');
	auto m = p.read_number<unsigned>();
	p.skip_char('-');
	auto d = p.read_number<unsigned>();

	return {
		std::chrono::year{y}, //
		std::chrono::month{m},
		std::chrono::day{d}
	};
}
} // namespace

namespace {
model::entry parse_entry(const tml::forest& forest)
{
	float kcal = 0;
	float mass = 0;
	uint32_t pcs = 0;
	std::u32string name;

	for (auto& c : forest) {
		if (c.value == kcal_word) {
			kcal = c.children.at(0).value.to_float();
		} else if (c.value == mass_word) {
			mass = c.children.at(0).value.to_float();
		} else if (c.value == pcs_word) {
			pcs = c.children.at(0).value.to_uint32();
		} else if (c.value == name_word) {
			name = utki::to_utf32(c.children.at(0).value.string);
		}
	}

	return {.name = std::move(name), .pcs = pcs, .mass = mass, .kcal = kcal};
}
} // namespace

namespace {
model::day parse_day(const tml::tree& tree)
{
	std::vector<model::entry> entries;

	for (auto& e : tree.children) {
		entries.push_back(parse_entry(e.children));
	}

	return {.date = parse_yyyy_mm_dd(tree.value.string), .entries = std::move(entries)};
}
} // namespace

namespace {
std::vector<model::day> parse_history(const tml::forest& forest)
{
	std::vector<model::day> history;

	for (auto& tree : forest) {
		history.push_back(parse_day(tree));
	}

	return history;
}
} // namespace

calslog::model::root calslog::model::read(const fsif::file& fi)
{
	auto forest = tml::read(fi);

	root r;

	for (auto& n : forest) {
		if (n.value == "foods"sv) {
			r.foods = parse_foods(n.children);
		} else if (n.value == "history"sv) {
			r.history = parse_history(n.children);
		}
	}

	return r;
}

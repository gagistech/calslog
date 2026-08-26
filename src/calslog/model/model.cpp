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

namespace {
std::string make_date_string(std::chrono::year_month_day d)
{
	char buf[16];
	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
	std::snprintf(
		buf, //
		sizeof buf, //
		"%04d-%02u-%02u", //
		static_cast<int>(d.year()), //
		static_cast<unsigned>(d.month()), //
		static_cast<unsigned>(d.day())
	);
	return buf;
} // make_date_string
} // namespace

namespace {
// creates a tree node with the given name and a single child leaf with the given value,
// i.e. <name>{<value>}
tml::tree make_key_value_node(std::string_view key, tml::leaf value)
{
	auto node = tml::tree(key);
	node.children.push_back(std::move(value));
	return node;
}
} // namespace

namespace {
tml::tree make_food_node(const model::food& f)
{
	auto node = tml::tree(utki::to_utf8(f.name));
	node.children.push_back(make_key_value_node(kcal_word, tml::leaf(f.kcal)));
	node.children.push_back(make_key_value_node(mass_word, tml::leaf(f.mass)));
	return node;
}
} // namespace

namespace {
tml::tree make_entry_node(const model::entry& e)
{
	auto node = tml::tree("e"sv);
	node.children.push_back(make_key_value_node(name_word, tml::leaf(utki::to_utf8(e.name))));
	node.children.push_back(make_key_value_node(kcal_word, tml::leaf(e.kcal)));
	node.children.push_back(make_key_value_node(pcs_word, tml::leaf(e.pcs)));
	node.children.push_back(make_key_value_node(mass_word, tml::leaf(e.mass)));
	return node;
}
} // namespace

namespace {
tml::tree make_day_node(const model::day& d)
{
	auto node = tml::tree(make_date_string(d.date));
	for (const auto& e : d.entries) {
		node.children.push_back(make_entry_node(e));
	}
	return node;
}
} // namespace

void calslog::model::write(const root& r, fsif::file& fi)
{
	auto foods = tml::tree("foods"sv);
	for (const auto& f : r.foods) {
		foods.children.push_back(make_food_node(f));
	}

	auto history = tml::tree("history"sv);
	for (const auto& d : r.history) {
		history.children.push_back(make_day_node(d));
	}

	tml::forest forest{std::move(foods), std::move(history)};

	tml::write(forest, fi);
}

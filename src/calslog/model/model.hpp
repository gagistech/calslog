#pragma once

#include <vector>
#include <map>
#include <chrono>
#include <limits>

namespace calslog::model{

using food_id_type = uint32_t;
constexpr food_id_type invalid_food_id = std::numeric_limits<food_id_type>::max();

struct entry{
    std::u32string name;
    float weight_grams;
    float kcal_per_100_grams;

    food_id_type food_id = invalid_food_id;
};

struct day{
    std::chrono::year_month_day date;
    std::vector<entry> entries;
};

struct food{
    float kcal_per_100_grams;
};

struct root{
    std::vector<day> history;

    day today;

    std::map<food_id_type, food> foods;
};

}

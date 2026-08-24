#pragma once

#include <vector>
#include <map>
#include <chrono>
#include <limits>

#include <fsif/file.hpp>

namespace calslog::model{

struct entry{
    std::u32string name;
    unsigned pcs; // number of pieces
    float mass; // mass of 1 piece in grams
    float kcal; // per 100 grams
};

struct day{
    std::chrono::year_month_day date;
    std::vector<entry> entries;
};

struct food{
    std::u32string name;
    float kcal; // per 100 grams
    float mass; // mass of 1 piece in grams
};

struct root{
    std::vector<day> history;

    day today;

    std::vector<food> foods;
};

root read(const fsif::file& fi);
void write(const root& r, fsif::file& fi);

}

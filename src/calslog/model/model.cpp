#include "model.hpp"

#include <tml/tree.hpp>

using namespace calslog;

/*
=========================
==== tml file format ====

foods{
    <name>{
        kcals{<kcals-per-100-grams>}
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

calslog::model::root calslog::model::read(const fsif::file& fi){
    auto forest = tml::read(fi);

    root r;

    // TODO:

    return r;
}

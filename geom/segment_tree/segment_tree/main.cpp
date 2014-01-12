#include "stdafx.h"
#include "primitives.h"
#include "range_tree.h"

int main()
{
    vector<point_t> points;
    vector<range_t> ranges;

/*
    for (size_t i = 0; i < 1000; ++i)
    {
        const coord_t x = rand();
        const coord_t y = rand();
        points.push_back(point_t(x, y));
        points.push_back(point_t(x, rand()));
    }


    for (size_t i = 0; i < 1000; ++i)
    {
        coord_t inf = rand();
        coord_t sup = rand();
        if (sup < inf)
            std::swap(inf, sup);

        ranges.push_back(range_t(inf, sup));
    }
*/


    for (size_t i = 0; i < 10; ++i)
        points.push_back(point_t(i,  5));


    const range_tree_t range_tree(points);

    auto ind = range_tree.query(range_t(2, 8), range_t(4, 9));

    vector<bool> returned;
    for (size_t i = 0; i < ranges.size(); ++i)
    {
        returned.resize(range_tree.points().size(), false);
        
        range_t x_range = ranges.at(i * 2 + 0);
        range_t y_range = ranges.at(i * 2 + 1);

        const auto indices = range_tree.query(x_range, y_range);
        BOOST_FOREACH(const auto index, indices)
        {
            returned.at(index) = true;
            const point_t point = range_tree.points().at(index);
            MY_ASSERT(point.x >= x_range.inf && point.x < x_range.sup && point.y >= y_range.inf && point.y < y_range.sup);
            int aaa = 6;
        }

        for (size_t index = 0; i < range_tree.points().size(); ++i)
        {
            const point_t point = range_tree.points().at(i);
            if (!returned.at(i))
                MY_ASSERT(point.x < x_range.inf || point.x >= x_range.sup || point.y < y_range.inf || point.y >= y_range.sup);
            int aaa = 6;
        }

    }
}
#include "stdafx.h"
#include "primitives.h"
#include "range_tree.h"

int main()
{
    vector<point_t> points;

    for (size_t i = 0; i < 1000; ++i)
    {
        points.push_back(point_t(rand(), rand()));
    }

    const range_tree_t range_tree(points);

    vector<range_t> ranges;
    for (size_t i = 0; i < 1000; ++i)
    {
        coord_t inf = rand();
        coord_t sup = rand();
        if (sup < inf)
            std::swap(inf, sup);

        ranges.push_back(range_t(inf, sup));
    }


    vector<bool> returned;
    for (size_t i = 0; i < ranges.size(); ++i)
    {
        returned.resize(range_tree.points().size(), false);
        
        range_t range = ranges.at(i);

        const auto indices = range_tree.query(range);
        BOOST_FOREACH(const auto index, indices)
        {
            returned.at(index) = true;
            const point_t point = range_tree.points().at(index);
            MY_ASSERT(point.x >= range.inf && point.x < range.sup);
            int aaa = 6;
        }

        for (size_t index = 0; i < range_tree.points().size(); ++i)
        {
            const point_t point = range_tree.points().at(i);
            if (!returned.at(i))
                MY_ASSERT(point.x < range.inf || point.x >= range.sup);
            int aaa = 6;
        }

    }
}
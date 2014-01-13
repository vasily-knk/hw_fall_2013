#pragma once

#include "range_tree.h"
#include "segment_tree.h"

struct windowing_t
{
    typedef vector<segment_t> segments_t;
    typedef unordered_set<size_t> indices_t;

    windowing_t(const segments_t &segments)
        : ranges_(extract_points(segments))
        , x_segments_(segments)
        , y_segments_(swap_xy(segments))
    {

    }

    indices_t query(const range_t &x, const range_t &y)
    {
        indices_t res;

        vector<segment_tree_t::range_its> results;
        results.push_back(x_segments_.query(segment_tree_t::query_t(x.inf, y)));
        results.push_back(x_segments_.query(segment_tree_t::query_t(x.sup, y)));
        results.push_back(y_segments_.query(segment_tree_t::query_t(y.inf, x)));
        results.push_back(y_segments_.query(segment_tree_t::query_t(y.sup, x)));

        auto inner = ranges_.query(x, y);
        BOOST_FOREACH(auto i, inner)
            res.insert(i / 2);

        BOOST_FOREACH(const auto &r, results)
            boost::copy(r, std::inserter(res, res.end()));

        
        return res;
    }

    const segments_t &segments() const
    {
        return x_segments_.segments();
    }

private:
    static segments_t swap_xy(const segments_t &segments)
    {
        segments_t res;
        boost::transform(segments, std::back_inserter(res),
            [](const segment_t &s)
        {
            return segment_t(point_t(s[0].y, s[0].x), point_t(s[1].y, s[1].x));
        });
        return res;
    }

    static range_tree_t::points_t extract_points(const segments_t &segments)
    {
        range_tree_t::points_t points;
        points.resize(segments.size() * 2);

        for (size_t i = 0; i < segments.size(); ++i)
        {
            points.at(i * 2 + 0) = segments.at(i)[0];
            points.at(i * 2 + 1) = segments.at(i)[1];
        }
        
        return points;
    }

private:
    range_tree_t ranges_;
    segment_tree_t x_segments_, y_segments_;
};
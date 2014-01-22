#pragma once

#include "primitives.h"
#include "tree.h"


inline range_t x_range(const segment_t &segment)
{
    return range_t(std::min(segment[0].x, segment[1].x), std::max(segment[0].x, segment[1].x));
}

inline range_t y_range(const segment_t &segment)
{
    return range_t(std::min(segment[0].y, segment[1].y), std::max(segment[0].y, segment[1].y));
}


inline coord_t value_for_x(const segment_t &segment, coord_t x)
{
    const range_t rg = x_range(segment);
    if (!rg.contains(x))
        throw std::logic_error("x value outside segment");

    if (segment[0].x == segment[1].x)
        return segment[0].y;

    const double ratio = double(x - segment[0].x) / double(segment[1].x - segment[0].x);
    const double offset = ratio * double(segment[1].y - segment[0].y);
    return segment[0].y + coord_t(offset);
}


bool point_to_the_left(const segment_t &segment, const point_t &point)
{
    const vector_t v1 = segment[1] - segment[0];
    const vector_t v2 = point - segment[0];

    return (v1 ^ v2) > 0;
}

bool compare_segments(const segment_t &s1, const segment_t &s2)
{
    const segment_t os1(min(s1), max(s1));
    const segment_t os2(min(s2), max(s2));

    const bool l1 = point_to_the_left(os1, os2[0]);
    const bool l2 = point_to_the_left(os1, os2[1]);

    if (l1 == l2)
        return l1;
    else
    {
        const bool r1 = point_to_the_left(os2, os1[0]);
        const bool r2 = point_to_the_left(os2, os1[1]);

        MY_ASSERT(r1 == r2);
        return !r1;
    }
}


struct segment_tree_t
{
    typedef vector<segment_t> segments_t;
    typedef uint32_t range_it;
    typedef vector<range_it> range_its;

    struct query_t
    {
        query_t (coord_t x, const range_t &y)
            : x(x)
            , y(y)
        { }

        coord_t x;
        range_t y;
    };

    segment_tree_t(const segments_t &ranges)
        : root_(build_tree(ranges))
        , segments_(ranges)
    {
        insert_segments();
        check(root_);
    }

    range_its query(const query_t &q) const
    {
        range_its dst;
        if (q.y.sup < q.y.inf)
            return dst;

        query(q, root_, dst);
        return dst;
    }

    uint32_t get_id(range_it it) const
    {
        return it;
    }

    const segments_t &segments() const 
    {
        return segments_;
    }

private:

    struct node_data_t
    {
        node_data_t(range_t const &interval)
            : interval(interval)
        {}

        range_t interval;
        range_its segments;
    };

    typedef node_base_t<node_data_t> node_t;
    typedef node_t::ptr node_ptr;

private:
    static vector<node_ptr> make_parents(const vector<node_ptr> &children)
    {
        vector<node_ptr> parents;

        node_ptr left;

        BOOST_FOREACH(node_ptr node, children)
        {
            if (!left)
                left = node;
            else
            {
                node_ptr right = node;
                range_t range(left->value().interval.inf, right->value().interval.sup);
                MY_ASSERT(range.inf <= range.sup);
                parents.push_back(make_shared<node_t>(range, left, right));

                left.reset();
            }
        }

        if (left)
        {
            range_t range(left->value().interval.inf, left->value().interval.sup);
            MY_ASSERT(range.inf <= range.sup);
            parents.push_back(make_shared<node_t>(range, left, node_ptr()));
        }

        return parents;
    }

    static node_ptr build_tree(const segments_t &segments)
    {
        std::set<coord_t> endpoints;
        BOOST_FOREACH(const auto &segment, segments)
        {
            endpoints.insert(segment2range(segment).inf);
            endpoints.insert(segment2range(segment).sup);
        }

        vector<node_ptr> nodes;

        for (auto it = endpoints.begin(); it != endpoints.end(); ++it)
        {
            nodes.push_back(make_shared<node_t>(range_t(*it, *it)));

            const auto next_it = std::next(it);
            if (next_it != endpoints.end())
                nodes.push_back(make_shared<node_t>(range_t(*it + 1, *next_it - 1)));
        }

        while(nodes.size() != 1)
        {
            MY_ASSERT(!nodes.empty());
            nodes = make_parents(nodes);
        }

        return nodes.front();
    }

    static range_t segment2range(const segment_t &segment) 
    {
        return x_range(segment);
    }

private:
    void insert_segment(range_it it, node_ptr node)
    {
        // can't have only right child
        MY_ASSERT(node->l() || !node->r());

        range_t interval = node->value().interval;
        range_t it_range = segment2range(segments_.at(it));

        if ((it_range & interval).is_empty())
        {
            // root has to intersect EVERY inserted segment
            MY_ASSERT(node != root_);
            return;
        }

        if (interval.inf >= it_range.inf && interval.sup <= it_range.sup)
        {
            // store the segment here
            node->value().segments.push_back(it);
        }
        else
        {
            if (node->l())
                insert_segment(it, node->l());
            if (node->r())
                insert_segment(it, node->r());
        }
    }

    void insert_segments()
    {
        for (auto it = segments_.begin(); it != segments_.end(); ++it)
            insert_segment(it - segments_.begin(), root_);

        sort_segments(root_);
    }

    void sort_segments(node_ptr node)
    {
        auto comp = [this, node](range_it it1, range_it it2) -> bool
        {
            return compare_segments(segments_.at(it1), segments_.at(it2));;
        };

        // maintaining segments order
        boost::sort(node->value().segments, comp);

        if (node->l())
            sort_segments(node->l());
        if (node->r())
            sort_segments(node->r());
    }

    void query(query_t q, node_ptr node, range_its &dst) const
    {
        const range_t interval = node->value().interval;
        if (q.x < interval.inf || q.x > interval.sup)
            return;

        // extraction
        auto comp = [this](range_it it, const point_t &point) -> bool
        {
            const segment_t os(geom::structures::min(segments_.at(it)), geom::structures::max(segments_.at(it)));
            const bool res1 = point_to_the_left(os, point);
            return res1;
        };

        const auto &segments = node->value().segments;
        const auto it1 = boost::lower_bound(segments, point_t(q.x, q.y.inf), comp);
        const auto it2 = boost::lower_bound(segments, point_t(q.x, q.y.sup), comp);

        std::copy(it1, it2, std::back_inserter(dst));

        if (node->l() && node->l()->value().interval.contains(q.x))
            query(q, node->l(), dst);
        else if (node->r())
            query(q, node->r(), dst);
    }

    static void check(node_ptr node)
    {
        // can't have only right child
        MY_ASSERT(node->l() || !node->r());
        const auto interval = node->value().interval;

        if (node->l() && node->r())
        {
            const auto int_l = node->l()->value().interval;
            const auto int_r = node->r()->value().interval;

            MY_ASSERT(int_l.inf == interval.inf);
            MY_ASSERT(int_r.sup == interval.sup);

            const auto intersection = int_l & int_r;
            MY_ASSERT(int_l.sup == int_r.inf - 1);
        }

        if (node->l())
            check(node->l());

        if (node->r())
            check(node->r());
    }

private:
    node_ptr root_;
    segments_t segments_;
};


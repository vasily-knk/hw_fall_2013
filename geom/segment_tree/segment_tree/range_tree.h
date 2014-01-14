#pragma once

#include "primitives.h"
#include "tree.h"

struct range_tree_t
{
    typedef vector<point_t> points_t;
    
    range_tree_t(const points_t &points)
        : points_(points)
        , subset_(prepare_subset())
        , root_(build_tree())
    {
        MY_ASSERT(ok());
    }

    vector<size_t> query(const range_t &x_range, const range_t &y_range) const
    {
        if (!root_)
            return vector<size_t>();
        
        node_t::ptr node = find_split_node(x_range);

        const vector<cascade_index_t> &y_indices = node->value().y_ordered;

        const size_t i1 = boost::lower_bound(y_indices, y_range.inf, comparator2_t(points_, false, true)) - y_indices.begin();
        const size_t i2 = boost::lower_bound(y_indices, y_range.sup, comparator2_t(points_, false, true)) - y_indices.begin();

        point_indices_t indices;

        if (node->is_leaf())
        {
            extract_indices(node, make_pair(i1, i2), indices);
        }
        else
        {
            run_left (node, x_range, i1, i2, OUT_ARG(indices));
            run_right(node, x_range, i1, i2, OUT_ARG(indices));
        }

        vector<size_t> result;
        boost::transform(indices, std::back_inserter(result), 
            [](const point_index_t &i) { return i.i; });

        return result;
    }

    const points_t &points() const
    {
        return points_;
    }

private:
    struct point_index_t
    {
        explicit point_index_t(size_t i)
            : i(i) 
        {}
        size_t i;
    };
    typedef vector<point_index_t> point_indices_t;
    
    // structure used for cascading
    struct cascade_index_t
    {
        point_index_t i;
        optional<size_t> l, r;

        cascade_index_t(const point_index_t &i)
            : i(i)
        {}
        
        operator const point_index_t&() const
        {
            return i;
        }

        optional<size_t> get_ptr(bool left) const
        {
            return left ? l : r;
        }
    };

    struct subset_t
    {
        vector<point_index_t> x_ordered;
        vector<cascade_index_t> y_ordered;
    };
    
    typedef node_base_t<subset_t> node_t;

    
    struct comparator_t
    {
        comparator_t(const points_t &points, bool x_coord) 
            : points_(&points) 
            , x_coord_(x_coord)
        {}

        bool operator()(point_index_t i1, point_index_t i2) const
        {
            const point_t &p1 = points_->at(i1.i);
            const point_t &p2 = points_->at(i2.i);
            if (x_coord_)
                return p1.x < p2.x || (p1.x == p2.x && p1.y < p2.y);
            else
                return p1.y < p2.y || (p1.y == p2.y && p1.x < p2.x);
        }

    private:
        const points_t *points_;
        bool x_coord_;
    };
    
    struct comparator2_t
    {
        comparator2_t(const points_t &points, bool x_coord, bool /*inf*/) 
            : points_(&points) 
            , x_coord_(x_coord)
        {}

        bool operator()(point_index_t i1, coord_t c2) const
        {
            const point_t &p1 = points_->at(i1.i);

            if (x_coord_)
                return p1.x < c2;
            else
                return p1.y < c2;
        }

    private:
        const points_t *points_;
        bool x_coord_;
    };

    subset_t prepare_subset() const 
    {
        subset_t s;
        s.x_ordered = prepare_sorted<point_index_t>(true );
        s.y_ordered = prepare_sorted<cascade_index_t>(false);
        return s;
    }
    
    template<typename T>
    vector<T> prepare_sorted(bool x_coord) const
    {
        vector<T> indices(points_.size(), point_index_t(0));
        for (size_t i = 0; i < indices.size(); ++i)
            indices.at(i) = point_index_t(i);

        boost::sort(indices, comparator_t(points_, x_coord));
        
        return indices;
    }

    node_t::ptr build_tree() const 
    {
        subset_t s(subset_);
        return build_tree(s);
    }

   
    pair<subset_t, subset_t> split_subset(subset_t &s) const
    {
        pair<subset_t, subset_t> result;

        const size_t size = s.x_ordered.size();
        const size_t med = size / 2;
        const point_index_t med_index = s.x_ordered.at(med);

        std::copy(s.x_ordered.begin(), s.x_ordered.begin() + med, std::back_inserter(result.first .x_ordered));
        std::copy(s.x_ordered.begin() + med, s.x_ordered.end()  , std::back_inserter(result.second.x_ordered));
        
        auto &left  = result.first .y_ordered;
        auto &right = result.second.y_ordered;
        
        comparator_t x_comp(points_, true);
        BOOST_FOREACH(const cascade_index_t &layered_index, s.y_ordered)
        {
            if (x_comp(layered_index.i, med_index))
                left.push_back(layered_index.i);
            else
                right.push_back(layered_index.i);
        }

        comparator_t y_comp(points_, false);
        size_t lptr = 0, rptr = 0;
        BOOST_FOREACH(cascade_index_t &layered_index, s.y_ordered)
        {
            MY_ASSERT(!layered_index.l && !layered_index.r);

            while (lptr < left .size() && y_comp(left .at(lptr), layered_index)) ++lptr;
            while (rptr < right.size() && y_comp(right.at(rptr), layered_index)) ++rptr;

            // check if linked item is the least one that is >= current
            MY_ASSERT((lptr == left .size() || (!y_comp(left .at(lptr), layered_index)) && (lptr == 0 || y_comp(left .at(lptr - 1), layered_index))));
            MY_ASSERT((rptr == right.size() || (!y_comp(right.at(rptr), layered_index)) && (rptr == 0 || y_comp(right.at(rptr - 1), layered_index))));

            layered_index.l = lptr;
            layered_index.r = rptr;
        }

        MY_ASSERT(check_subset(result.first ));
        MY_ASSERT(check_subset(result.second));
        MY_ASSERT(abs(int64_t(result.first.x_ordered.size()) - int64_t(result.second.x_ordered.size())) <= 1);
        
        return result;
    }
    
    node_t::ptr build_tree(const subset_t &old_s) const
    {
        subset_t s = old_s;
        MY_ASSERT(s.x_ordered.size() == s.y_ordered.size());

        node_t::ptr l, r;
        if (s.x_ordered.size() > 1)
        {
            auto split = split_subset(s);
            
            l = build_tree(split.first );
            r = build_tree(split.second);
        }
        
        return node_t::create(s, l, r);
    }

    node_t::ptr find_split_node(const range_t &range) const
    {
        const comparator2_t inf_comp(points_, true, true);
        const comparator2_t sup_comp(points_, true, false);

        node_t::ptr node = root_;
        while (!node->is_leaf())
        {
            const point_index_t index = node_x(node);
            if (sup_comp(index, range.sup) && !inf_comp(index, range.inf))
                break;

            node = (!sup_comp(index, range.sup)) ? node->l() : node->r();
        }
        return node;
    }

    point_index_t node_x(node_t::ptr node) const
    {
        const auto &xs = node->value().x_ordered;
        return xs.at(xs.size() / 2);
    }

    const point_t &get_point(const point_index_t &index) const
    {
        return points_.at(index.i);
    }

    static pair<size_t, size_t> sublimits(node_t::ptr node, const pair<size_t, size_t> &parent_indices, bool left) 
    {
        node_t::ptr child = left ? node->l() : node->r();
        const auto &parent_subset = node->value().y_ordered;
        const auto &child_subset = child->value().y_ordered;

        pair<size_t, size_t> child_indices;
        if (parent_indices.first == parent_subset.size())
            child_indices.first = child_subset.size();
        else 
            child_indices.first = *parent_subset.at(parent_indices.first).get_ptr(left);

        if (parent_indices.second == parent_subset.size())
            child_indices.second = child_subset.size();
        else 
            child_indices.second = *parent_subset.at(parent_indices.second).get_ptr(left);

    
        return child_indices;
    }

    static void extract_indices(node_t::ptr node, const pair<size_t, size_t> &limits, point_indices_t &out_indices) 
    {
        const auto start_it = node->value().y_ordered.begin();
        std::copy(start_it + limits.first, start_it + limits.second, std::back_inserter(out_indices));
    }


    void run_left(node_t::ptr start, const range_t &range, size_t ibegin, size_t iend, point_indices_t &out_indices) const
    {
        const comparator2_t comp(points_, true, true);

        pair<size_t, size_t> limits = sublimits(start, make_pair(ibegin, iend), true);
        node_t::ptr node = start->l();


        while(!node->is_leaf())
        {
            bool step_left;
            
            const point_index_t index = node_x(node);
            // x_v >= x
            if (!comp(index, range.inf))
            {
                extract_indices(node->r(), sublimits(node, limits, false), out_indices);

                step_left = true;
            }
            else
                step_left = false;

            limits = sublimits(node, limits, step_left);
            node = step_left ? node->l() : node->r();
        }
        const point_index_t index = node_x(node);

        if (!comp(index, range.inf))
            extract_indices(node, limits, out_indices);
    }

    void run_right(node_t::ptr start, const range_t &range, size_t ibegin, size_t iend, point_indices_t &out_indices) const
    {
        const comparator2_t comp(points_, true, true);

        pair<size_t, size_t> limits = sublimits(start, make_pair(ibegin, iend), false);
        node_t::ptr node = start->r();


        while(!node->is_leaf())
        {
            bool step_left;

            const point_index_t index = node_x(node);
            // x_v < x'
            if (comp(index, range.sup))
            {
                extract_indices(node->l(), sublimits(node, limits, true), out_indices);

                step_left = false;
            }
            else
                step_left = true;

            limits = sublimits(node, limits, step_left);
            node = step_left ? node->l() : node->r();
        }
        const point_index_t index = node_x(node);

        if (comp(index, range.sup))
            extract_indices(node, limits, out_indices);
    }

private:
    // checks

    bool check_subset(const subset_t &s) const
    {
        MY_ASSERT(s.x_ordered.size() == s.y_ordered.size());
        MY_ASSERT(boost::is_sorted(s.x_ordered, comparator_t(points_, true )));
        MY_ASSERT(boost::is_sorted(s.y_ordered, comparator_t(points_, false)));
        return true;
    }

    bool check_cascades(node_t::ptr node) const
    {
        if (!node->l() && !node->r())
            return true;

        const vector<cascade_index_t> &layered_indices = node->value().y_ordered;
        const vector<cascade_index_t> &left  = node->l()->value().y_ordered;
        const vector<cascade_index_t> &right = node->r()->value().y_ordered;

        comparator_t comp(points_, false);
        BOOST_FOREACH(const cascade_index_t &layered_index, layered_indices)
        {
            if (layered_index.l < left.size())
            {
                const size_t p = *layered_index.l;
                MY_ASSERT(!comp(left.at(p), layered_index));
                MY_ASSERT(p == 0 || comp(left.at(p - 1), layered_index))
            }

            if (layered_index.r < right.size())
            {
                const size_t p = *layered_index.r;
                MY_ASSERT(!comp(right.at(p), layered_index));
                MY_ASSERT(p == 0 || comp(right.at(p - 1), layered_index))
            }
        }

        check_cascades(node->l());
        check_cascades(node->r());

        return true;
    }

    bool ok() const
    {
        MY_ASSERT(root_ || points_.empty());
        check_subset(subset_);
        check_cascades(root_);
        return true;
    }

private:    
    points_t points_;
    subset_t subset_;
    node_t::ptr root_;
};
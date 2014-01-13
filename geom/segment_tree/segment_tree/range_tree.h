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
        
        deque<node_t::ptr> nodes;

        node_t::ptr node = find_split_node(x_range);
        if (node->is_leaf())
        {
            nodes.push_back(node);
        }
        else
        {
            run_left (node, x_range, OUT_ARG(nodes));
            run_right(node, x_range, OUT_ARG(nodes));
        }

        return select_indices(nodes, y_range);
    }

    const points_t &points() const
    {
        return points_;
    }

private:
    struct index_t
    {
        explicit index_t(size_t i)
            : i(i) 
        {}
        size_t i;
    };
    typedef vector<index_t> indices_t;
    
    // structure used for cascading
    struct layered_index_t
    {
        index_t i;
        optional<size_t> l, r;

        layered_index_t(const index_t &i)
            : i(i)
        {}
        
        operator const index_t&() const
        {
            return i;
        }
    };
    
    struct subset_t
    {
        vector<index_t> x;
        vector<layered_index_t> y;
    };
    
    struct assoc_struct_t 
    {
        assoc_struct_t (subset_t const &s)
            : s(s)
        {

        }

        subset_t s;
    };
    typedef node_base_t<assoc_struct_t> node_t;

    
    struct comparator_t
    {
        comparator_t(const points_t &points, bool x_coord) 
            : points_(&points) 
            , x_coord_(x_coord)
        {}

        bool operator()(index_t i1, index_t i2) const
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

        bool operator()(index_t i1, coord_t c2) const
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
        s.x = prepare_sorted<index_t>(true );
        s.y = prepare_sorted<layered_index_t>(false);
        return s;
    }
    
    template<typename T>
    vector<T> prepare_sorted(bool x_coord) const
    {
        vector<T> indices(points_.size(), index_t(0));
        for (size_t i = 0; i < indices.size(); ++i)
            indices.at(i) = index_t(i);

        boost::sort(indices, comparator_t(points_, x_coord));
        
        return indices;
    }

    node_t::ptr build_tree() const 
    {
        subset_t s(subset_);
        return build_tree(s);
    }

    bool check_subset(const subset_t &s) const
    {
        MY_ASSERT(s.x.size() == s.y.size());
        MY_ASSERT(boost::is_sorted(s.x, comparator_t(points_, true )));
        MY_ASSERT(boost::is_sorted(s.y, comparator_t(points_, false)));
        return true;
    }

    bool check_cascades(node_t::ptr node) const
    {
        if (!node->l() && !node->r())
            return true;

        const vector<layered_index_t> &layered_indices = node->value().s.y;
        const vector<layered_index_t> &left  = node->l()->value().s.y;
        const vector<layered_index_t> &right = node->r()->value().s.y;

        comparator_t comp(points_, false);
        BOOST_FOREACH(const layered_index_t &layered_index, layered_indices)
        {
            if (layered_index.l)
            {
                const size_t p = *layered_index.l;
                MY_ASSERT(!comp(left.at(p), layered_index));
                MY_ASSERT(p == 0 || comp(left.at(p - 1), layered_index))
            }

            if (layered_index.r)
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
    
    pair<subset_t, subset_t> split_subset(subset_t &s) const
    {
        pair<subset_t, subset_t> result;

        const size_t size = s.x.size();
        const size_t med = size / 2;
        const index_t med_index = s.x.at(med);

        std::copy(s.x.begin(), s.x.begin() + med, std::back_inserter(result.first .x));
        std::copy(s.x.begin() + med, s.x.end()  , std::back_inserter(result.second.x));

        
        auto &left  = result.first .y;
        auto &right = result.second.y;
        
        comparator_t x_comp(points_, true);
        BOOST_FOREACH(const layered_index_t &layered_index, s.y)
        {
            if (x_comp(layered_index.i, med_index))
                left.push_back(layered_index.i);
            else
                right.push_back(layered_index.i);
        }

        comparator_t y_comp(points_, false);
        size_t lptr = 0, rptr = 0;
        BOOST_FOREACH(layered_index_t &layered_index, s.y)
        {
            MY_ASSERT(!layered_index.l && !layered_index.r);

            while (lptr < left .size() && y_comp(left .at(lptr), layered_index)) ++lptr;
            while (rptr < right.size() && y_comp(right.at(rptr), layered_index)) ++rptr;

            // check if linked item is the least one that is >= current
            MY_ASSERT(lptr == left .size() || (!y_comp(left .at(lptr), layered_index) && (lptr == 0 || y_comp(left .at(lptr - 1), layered_index))));
            MY_ASSERT(rptr == right.size() || (!y_comp(right.at(rptr), layered_index) && (rptr == 0 || y_comp(right.at(rptr - 1), layered_index))));

            if (lptr < left.size())
                layered_index.l = lptr;

            if (rptr < right.size())
                layered_index.r = rptr;
        }

        MY_ASSERT(check_subset(result.first ));
        MY_ASSERT(check_subset(result.second));
        MY_ASSERT(abs(int64_t(result.first.x.size()) - int64_t(result.second.x.size())) <= 1);
        
        return result;
    }
    
    node_t::ptr build_tree(const subset_t &old_s) const
    {
        subset_t s = old_s;
        MY_ASSERT(s.x.size() == s.y.size());

        node_t::ptr l, r;
        if (s.x.size() > 1)
        {
            auto split = split_subset(s);
            
            l = build_tree(split.first );
            r = build_tree(split.second);
        }
        
        return node_t::create(assoc_struct_t(s), l, r);
    }

    node_t::ptr find_split_node(const range_t &range) const
    {
        const comparator2_t inf_comp(points_, true, true);
        const comparator2_t sup_comp(points_, true, false);

        node_t::ptr node = root_;
        while (!node->is_leaf())
        {
            const index_t index = node_x(node);
            if (sup_comp(index, range.sup) && !inf_comp(index, range.inf))
                break;

            node = (!sup_comp(index, range.sup)) ? node->l() : node->r();
        }
        return node;
    }

    index_t node_x(node_t::ptr node) const
    {
        const auto &xs = node->value().s.x;
        return xs.at(xs.size() / 2);
    }

    const point_t &get_point(const index_t &index) const
    {
        return points_.at(index.i);
    }

    void run_left(node_t::ptr start, const range_t &range, deque<node_t::ptr> &out_nodes) const
    {
        const comparator2_t comp(points_, true, true);

        node_t::ptr node = start->l();
        while(!node->is_leaf())
        {
            const index_t index = node_x(node);
            // x_v >= x
            if (!comp(index, range.inf))
            {
                out_nodes.push_front(node->r());
                node = node->l();
            }
            else
                node = node->r();
        }
        const index_t index = node_x(node);
        if (!comp(index, range.inf))
            out_nodes.push_front(node);
    }

    void run_right(node_t::ptr start, const range_t &range, deque<node_t::ptr> &out_nodes) const
    {
        const comparator2_t comp(points_, true, false);

        node_t::ptr node = start->r();
        while(!node->is_leaf())
        {
            const index_t index = node_x(node);
            // x_v < x'
            if (comp(index, range.sup))
            {
                out_nodes.push_back(node->l());
                node = node->r();
            }
            else
                node = node->l();
        }
        const index_t index = node_x(node);
        if (comp(index, range.sup))
            out_nodes.push_back(node);
    }

    vector<size_t> select_indices(const deque<node_t::ptr> &nodes, const range_t &y_range) const
    {
        vector<size_t> result;
        for (auto it = nodes.begin(); it != nodes.end(); ++it)
        {
            node_t::ptr node = *it;

            const vector<layered_index_t> &y_indices = node->value().s.y;

            const auto it1 = boost::lower_bound(y_indices, y_range.inf, comparator2_t(points_, false, true));
            const auto it2 = boost::lower_bound(y_indices, y_range.sup, comparator2_t(points_, false, true));
            
            std::transform(it1, it2, std::back_inserter(result),
                [](const index_t &index) { return index.i; } );
        }
        return result;
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
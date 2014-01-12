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

    vector<size_t> query(range_t range) const
    {
        if (!root_)
            return vector<size_t>();
        
        deque<node_t::ptr> nodes;

        node_t::ptr node = find_split_node(range);
        if (node->is_leaf())
        {
            const assoc_struct_t &as = node->value();
            nodes.push_back(node);
        }
        else
        {
            run_left (node, range, OUT_ARG(nodes));
            run_right(node, range, OUT_ARG(nodes));
        }

        return select_indices(nodes);
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
    struct subset_t
    {
        indices_t x, y;
    };
    typedef indices_t::const_iterator it_t;
    
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
        comparator2_t(const points_t &points, bool x_coord, bool inf) 
            : points_(&points) 
            , x_coord_(x_coord)
            , inf_(inf)
        {}

        bool operator()(index_t i1, coord_t c2) const
        {
            const point_t &p1 = points_->at(i1.i);

            if (x_coord_)
                return p1.x < c2 || p1.x == c2 && !inf_;
            else
                return p1.y < c2 || p1.y == c2 && !inf_;
        }
    private:
        const points_t *points_;
        bool x_coord_;
        bool inf_;
    };

    subset_t prepare_subset() const 
    {
        subset_t s;
        s.x = prepare_sorted(true );
        s.y = prepare_sorted(false);
        return s;
    }
    
    indices_t prepare_sorted(bool x_coord) const
    {
        indices_t indices(points_.size(), index_t(0));
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
    
    pair<subset_t, subset_t> split_subset(const subset_t &s) const
    {
        pair<subset_t, subset_t> result;

        const size_t size = s.x.size();
        const size_t med = size / 2;
        const index_t med_index = s.x.at(med);

        std::copy(s.x.begin(), s.x.begin() + med, std::back_inserter(result.first .x));
        std::copy(s.x.begin() + med, s.x.end()  , std::back_inserter(result.second.x));

        comparator_t x_comp(points_, true);
        BOOST_FOREACH(const index_t &index, s.y)
        {
            if (x_comp(index, med_index))
                result.first.y.push_back(index);
            else
                result.second.y.push_back(index);
        }


        MY_ASSERT(check_subset(result.first ));
        MY_ASSERT(check_subset(result.second));
        MY_ASSERT(abs(int64_t(result.first.x.size()) - int64_t(result.second.x.size())) <= 1);
        
        return result;
    }
    
    node_t::ptr build_tree(const subset_t &s) const
    {
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
        const indices_t &xs = node->value().s.x;
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
                out_nodes.push_back(node->r());
                node = node->l();
            }
            else
                node = node->r();
        }
        const index_t index = node_x(node);
        if (!comp(index, range.inf))
            out_nodes.push_back(node);
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
                out_nodes.push_front(node->l());
                node = node->r();
            }
            else
                node = node->l();
        }
        const index_t index = node_x(node);
        if (comp(index, range.sup))
            out_nodes.push_front(node);
    }

    vector<size_t> select_indices(const deque<node_t::ptr> &nodes) const
    {
        vector<size_t> result;
        for (auto it = nodes.begin(); it != nodes.end(); ++it)
        {
            node_t::ptr node = *it;
            
            boost::transform(node->value().s.y, std::back_inserter(result),
                [](const index_t &index) { return index.i; } );
        }
        return result;
    }

    bool ok() const
    {
        MY_ASSERT(root_ || points_.empty());
        check_subset(subset_);
        return true;
    }

private:    
    points_t points_;
    subset_t subset_;
    node_t::ptr root_;
};
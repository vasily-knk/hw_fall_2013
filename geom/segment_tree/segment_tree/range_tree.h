#pragma once

#include "primitives.h"
#include "tree.h"

struct range_tree_t
{
    typedef vector<point_t> points_t;
    typedef size_t index_t;
    typedef vector<index_t> indices_t;

    range_tree_t(const points_t &points)
        : points_(points)
        , x_sorted_(prepare_sorted(true ))
        , y_sorted_(prepare_sorted(false))
        , root_(build_tree())
    {
        MY_ASSERT(ok());
    }

    indices_t query(range_t range) const
    {
        if (!root_)
            return indices_t();
        
        deque<node_t::ptr> nodes;

        node_t::ptr node = find_split_node(range);
        const size_t d1 = node->value().i_begin - x_sorted_.begin();
        const size_t d2 = node->value().i_end   - x_sorted_.begin();
        if (node->is_leaf())
        {
            const assoc_struct_t &as = node->value();
            MY_ASSERT(as.i_end - as.i_begin == 1);
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
    typedef indices_t::const_iterator it_t;
    
    struct assoc_struct_t 
    {
        assoc_struct_t(it_t i_begin, it_t i_end) 
            : i_begin(i_begin)
            , i_end(i_end)
        {
            MY_ASSERT(i_begin < i_end);
        }
        it_t i_begin, i_end;
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
            const point_t &p1 = points_->at(i1);
            const point_t &p2 = points_->at(i2);
            return x_coord_ ? p1.x < p2.x : p1.y < p2.y;
        }

        bool operator()(index_t i1, coord_t c2) const
        {
            const point_t &p1 = points_->at(i1);
            return x_coord_ ? p1.x < c2 : p1.y < c2;
        }

    private:
        const points_t *points_;
        bool x_coord_;
    };

    indices_t prepare_sorted(bool x_coord) const
    {
        indices_t indices(points_.size());
        for (index_t i = 0; i < indices.size(); ++i)
            indices.at(i) = i;

        boost::sort(indices, comparator_t(points_, x_coord));
        
        return indices;
    }

    node_t::ptr build_tree() const 
    {
        if (x_sorted_.empty())
            return node_t::ptr();

        return build_tree(x_sorted_.begin(), x_sorted_.end());
    }

    node_t::ptr build_tree(it_t i_begin, it_t i_end) const
    {
        const size_t d1 = i_begin - x_sorted_.begin();
        const size_t d2 = i_end   - x_sorted_.begin();
        MY_ASSERT(i_begin < i_end);

        
        node_t::ptr l, r;
        if (i_end - i_begin > 1)
        {
            const it_t i_med = i_begin + (i_end - i_begin) / 2;
            const size_t dm = i_med - x_sorted_.begin();
            l = build_tree(i_begin, i_med);
            r = build_tree(i_med, i_end);
        }
        
        return node_t::create(assoc_struct_t(i_begin, i_end), l, r);
    }

    node_t::ptr find_split_node(const range_t &range) const
    {
        const comparator_t comp(points_, true);

        node_t::ptr node = root_;
        while (!node->is_leaf())
        {
            const index_t index = node_x(node);
            if (comp(index, range.sup) && !comp(index, range.inf))
                break;

            node = (!comp(index, range.sup)) ? node->l() : node->r();
        }
        return node;
    }

    index_t node_x(node_t::ptr node) const
    {
        return *(node->value().i_begin + (node->value().i_end - node->value().i_begin) / 2);
    }

    void run_left(node_t::ptr start, const range_t &range, deque<node_t::ptr> &out_nodes) const
    {
        const comparator_t comp(points_, true);

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
        const comparator_t comp(points_, true);

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

    indices_t select_indices(const deque<node_t::ptr> &nodes) const
    {
        indices_t indices;
        for (auto it = nodes.begin(); it != nodes.end(); ++it)
        {
            node_t::ptr node = *it;
            std::copy(node->value().i_begin, node->value().i_end, std::back_inserter(indices));
        }
        return indices;
    }

    bool ok() const
    {
        MY_ASSERT(root_ || points_.empty());
        MY_ASSERT(points_.size() == x_sorted_.size());
        MY_ASSERT(points_.size() == y_sorted_.size());
        return true;
    }

private:    
    points_t points_;
    indices_t x_sorted_, y_sorted_;
    node_t::ptr root_;
};
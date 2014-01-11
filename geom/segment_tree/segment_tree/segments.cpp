#include "primitives.h"
#include "tree.h"

struct segment_t
{
	segment_t(point_t const &p1, point_t const &p2)
		: p1(p1)
		, p2(p2)
	{}
	
	range_t x_range() const { return range_t(std::min(p1.x, p2.x), std::max(p1.x, p2.x)); }
	range_t y_range() const { return range_t(std::min(p1.y, p2.y), std::max(p1.y, p2.y)); }

    bool lower(const segment_t &other) const
    {
        return true; // fixme
    }

	point_t p1, p2;
};

inline coord_t value_for_x(const segment_t &segment, coord_t x)
{
    const range_t rg = segment.x_range();
    if (!rg.contains(x))
        throw std::logic_error("x value outside segment");

    if (segment.p1.x == segment.p2.x)
        return segment.p1.y;

    const double ratio = double(x - segment.p1.x) / double(segment.p2.x - segment.p1.x);
    const double offset = ratio * double(segment.p2.y - segment.p1.y);
    return segment.p1.y + coord_t(offset);
}

struct segment_tree_base
{
    typedef vector<segment_t> segments_t;
    typedef segments_t::const_iterator range_it;
    typedef vector<range_it> range_its;

    typedef coord_t query_t;

	segment_tree_base(const segments_t &ranges)
		: root_(build_tree(ranges))
		, segments_(ranges)
	{
        insert_segments();
		check(root_);
	}

	range_its query(query_t q) const
	{
		range_its dst;
        query(q, root_, dst);
        return dst;
	}

    uint32_t get_id(range_it it) const
    {
        return it - segments_.begin();
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
				assert(range.inf <= range.sup);
				parents.push_back(make_shared<node_t>(range, left, right));

				left.reset();
			}
		}

		if (left)
		{
			range_t range(left->value().interval.inf, left->value().interval.sup);
			assert(range.inf <= range.sup);
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
			assert(!nodes.empty());
			nodes = make_parents(nodes);
		}

		return nodes.front();
	}

    static range_t segment2range(const segment_t &segment) 
    {
        return segment.x_range();
    }

private:
	void insert_segment(range_it it, node_ptr node)
	{
		// can't have only right child
		assert(node->l() || !node->r());

		range_t interval = node->value().interval;
        range_t it_range = segment2range(*it);

		if ((it_range & interval).is_empty())
		{
			// root has to intersect EVERY inserted segment
			assert(node != root_);
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
        for (range_it it = segments_.begin(); it != segments_.end(); ++it)
			insert_segment(it, root_);

        sort_segments(root_);
	}

    void sort_segments(node_ptr node)
    {
        auto comp = [this, node](range_it it1, range_it it2) -> bool
        {
            const coord_t x = node->value().interval.inf;
            const coord_t y1 = value_for_x(*it1, x);
            const coord_t y2 = value_for_x(*it2, x);

            return y1 < y2;
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
		if (q < interval.inf || q > interval.sup)
			return;

        // extraction
        auto comp = [this](range_it it, const point_t &point)
        {
            return value_for_x(*it, point.x) < point.y;
        };

       
        boost::copy(node->value().segments, std::back_inserter(dst));

		if (node->l() && node->l()->value().interval.contains(q))
			query(q, node->l(), dst);
		else if (node->r())
			query(q, node->r(), dst);
	}

	static void check(node_ptr node)
	{
		// can't have only right child
		assert(node->l() || !node->r());
		const auto interval = node->value().interval;

		if (node->l() && node->r())
		{
			const auto int_l = node->l()->value().interval;
			const auto int_r = node->r()->value().interval;

			assert(int_l.inf == interval.inf);
			assert(int_r.sup == interval.sup);

			const auto intersection = int_l & int_r;
			assert(int_l.sup == int_r.inf - 1);
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


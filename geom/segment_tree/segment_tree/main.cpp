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

struct segment_tree
{
	typedef uint32_t range_id;

	segment_tree(vector<range_t> ranges)
		: root_(build_tree(ranges))
		, ranges_(ranges)
	{
		insert_segments();
		check(root_);
	}

	vector<range_t> const &ranges() const
	{
		return ranges_;
	}

	vector<range_id> query(coord_t q) const
	{
		vector<range_id> res;
		query_impl(q, root_, res);
		return res;
	}

private:
    typedef vector<range_t>::const_iterator range_it;
	
	struct node_data_t
	{
		node_data_t(range_t const &interval)
			: interval(interval)
		{}

		range_t interval;
		vector<range_it> segments;
	};
	
	typedef node_base_t<node_data_t> node_t;
	typedef node_t::ptr node_ptr;

private:
	static vector<node_ptr> make_parents(vector<node_ptr> children)
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

	static node_ptr build_tree(vector<range_t> const &ranges)
	{
		std::set<coord_t> endpoints;
		BOOST_FOREACH(range_t const& range, ranges)
		{
			endpoints.insert(range.inf);
			endpoints.insert(range.sup);
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

private:
	void insert_segment(range_it it, node_ptr node)
	{
		// can't have only right child
		assert(node->l() || !node->r());

		range_t interval = node->value().interval;

		if ((*it & interval).is_empty())
		{
			// root has to intersect EVERY inserted segment
			assert(node != root_);
			return;
		}

		if (interval.inf >= it->inf && interval.sup <= it->sup)
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
        for (range_it it = ranges_.begin(); it != ranges_.end(); ++it)
			insert_segment(it, root_);
	}

	void query_impl(coord_t q, node_ptr node, vector<range_id> &out_ids) const
	{
		const range_t interval = node->value().interval;
		if (q < interval.inf || q > interval.sup)
			return;

        BOOST_FOREACH(range_it it, node->value().segments)
            out_ids.push_back(it - ranges_.begin());

		if (node->l() && node->l()->value().interval.contains(q))
			query_impl(q, node->l(), out_ids);
		else if (node->r())
			query_impl(q, node->r(), out_ids);
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
	vector<range_t> ranges_;
};



int main()
{
	vector<range_t> segments;

	segment_tree st(segments);
	auto res = st.query(60);

	return 0;
}
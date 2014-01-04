#include "primitives.h"
#include "tree.h"

typedef range_t segment_t;

struct segment_tree
{
	typedef uint32_t segment_id;

	segment_tree(vector<segment_t> segments)
		: root_(build_tree(segments))
		, segments_(segments)
	{
		insert_segments();
		check(root_);
	}

	vector<segment_t> const &segments() const
	{
		return segments_;
	}

	vector<segment_id> query(coord_t q) const
	{
		vector<segment_id> res;
		query_impl(q, root_, res);
		return res;
	}

private:
	
	struct node_data_t
	{
		node_data_t(range_t const &interval)
			: interval(interval)
		{

		}

		range_t interval;
		vector<segment_id> segments;
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
	void insert_segment(segment_id id, node_ptr node)
	{
		// can't have only right child
		assert(node->l() || !node->r());

		segment_t segment = segments_.at(id);
		range_t interval = node->value().interval;

		if ((segment & interval).is_empty())
		{
			// root has to intersect EVERY inserted segment
			assert(node != root_);
			return;
		}

		if (interval.inf >= segment.inf && interval.sup <= segment.sup)
		{
			// store the segment here
			node->value().segments.push_back(id);
		}
		else
		{
			if (node->l())
				insert_segment(id, node->l());
			if (node->r())
				insert_segment(id, node->r());
		}
	}

	void insert_segments()
	{
		for (uint32_t i = 0; i < segments_.size(); ++i)
			insert_segment(i, root_);
	}

	void query_impl(coord_t q, node_ptr node, vector<segment_id> &out_ids) const
	{
		const range_t interval = node->value().interval;
		if (q < interval.inf || q > interval.sup)
			return;

		boost::copy(node->value().segments, std::back_inserter(out_ids));

		if (node->l() && node->l()->value().interval.contains(q))
			query_impl(q, node->l(), out_ids);
		else if (node->r())
			query_impl(q, node->r(), out_ids);
	}

	void check(node_ptr node)
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
	vector<segment_t> segments_;
};



int main()
{
	vector<segment_t> segments;
	segments.push_back(segment_t(0, 20));
	segments.push_back(segment_t(10, 30));
	segments.push_back(segment_t(20, 30));
	segments.push_back(segment_t(5, 45));
	segments.push_back(segment_t(50, 60));

	segment_tree st(segments);
	auto res = st.query(60);

	return 0;
}
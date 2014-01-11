#pragma once

template<typename T>
struct node_base_t
{
	typedef shared_ptr<node_base_t> ptr;
	typedef T value_type;

    static ptr create(value_type value, ptr left = ptr(), ptr right = ptr())
    {
        return make_shared<node_base_t>(value, left, right);
    }

	node_base_t(value_type value, ptr left = ptr(), ptr right = ptr())
		: value_(value)
		, left_(left)
		, right_(right)
	{
		
	}

	value_type &value() { return value_; }
	ptr l() const { return left_ ; }
	ptr r() const { return right_; }

	bool is_leaf() const { return !left_ && !right_; }

private:
	value_type value_;
	ptr left_, right_;
};




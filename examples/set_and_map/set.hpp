# ifndef SET_H
# define SET_H

# include <concepts>
# include <initializer_list>
# include <ostream>

# include <tree/config.hpp>

# include <tree/avl_tree.hpp>

// NOTE: binary_tree_node and trees are implemented such a way that iterators
// are second class cityzens here and get invalidated after the container gets
// mutated. Instead of iterators all underlying implementation uses node** and
// stack<node**>. To make iterators first class cityzens first nodes must
// carry parrent node pointers. All code using nodes must be carefully rewriten.

namespace tree {

template <typename T, template <typename> typename Container = avl_tree>
// requires is_tree_v <Container <T>>
struct set : protected Container <T> {
public:
	using tree = Container <T>;
	using node = tree::node;
	using node_link = tree::node_link;
	using const_node_link = tree::const_node_link;
	using value_type = T;
	using tree::less_than;
	using iterator = tree::iterator;
	using const_iterator = tree::const_iterator;

public:
	using tree::dump_invariant;
	using tree::dump_sorted;
	using tree::make_empty;
	using tree::root;
	using tree::size;
	using tree::empty;
	using tree::internal_path_length;
	// using tree::insert;
	using tree::remove;
	using tree::contains;

	using tree::begin;
	using tree::cbegin;
	using tree::end;
	using tree::cend;
	using tree::find;

	set () = default;

	template <std::convertible_to <T> U>
	set (std::initializer_list <U> list) : tree () {
		for (const U & e : list) {
			insert (e);
		}
	}

	bool operator== (const set & other) const {
		return
			   static_cast <const tree &> (* this)
			== static_cast <const tree &> (other)
		;
	}

	operator vector <T> () const {
		vector <T> v;
		v.reserve (size ());

		dump_sorted (std::back_inserter (v));

		return v;
	}

	friend std::ostream & operator<< (std::ostream & os, const set & tree)
		requires requires (std::ostream & os, T t) {
			{ os << t } -> std::convertible_to <std::ostream &>;
		}
	{
		return os << static_cast <const set::tree &> (tree);
	}

public:
	template <typename U>
	requires std::convertible_to <U, T>
	bool insert (U && value) {
		bool inserting = false;

		if (false == contains (value)) {
			this->tree::insert (std::forward <U> (value));
			inserting = true;
		}	// after moveing value we can't use it to get an iterator,
			// so insert should return one.

		return inserting;
	}
};

} // end namespace tree

# endif // SET_H

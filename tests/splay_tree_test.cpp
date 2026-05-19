//   ____                _       _                                       _           _
//  / ___| ___ _ __ ___ (_)_ __ (_)       __ _  ___ _ __   ___ _ __ __ _| |_ ___  __| |
// | |  _ / _ \ '_ ` _ \| | '_ \| |_____ / _` |/ _ \ '_ \ / _ \ '__/ _` | __/ _ \/ _` |
// | |_| |  __/ | | | | | | | | | |_____| (_| |  __/ | | |  __/ | | (_| | ||  __/ (_| |
//  \____|\___|_| |_| |_|_|_| |_|_|      \__, |\___|_| |_|\___|_|  \__,_|\__\___|\__,_|
//                                       |___/

# include <gtest/gtest.h>
# include <stddef.h>
# include <vector>
# include <algorithm>
# include <random>
# include <set>
# include <string>
# include <functional>
# include <limits>
# include <iostream>
# include <numeric>

# include <tree/splay_tree.hpp>
using namespace tree;



/**
 * Fixture: SplayTreeTest
 * Provides a concrete environment for verifying splay tree invariants.
 */
class SplayTreeTest : public ::testing::Test {
protected:
	using TreeType = splay_tree<int>;

	void SetUp() override {
		std::random_device rd;
		gen.seed(rd());
	}

	std::mt19937 gen;

	/**
	 * Lemma 1: BST Property
	 * For any node n, all nodes in the left subtree are less than n,
	 * and all nodes in the right subtree are greater than n.
	 */
	bool VerifyBSTInvariant(const TreeType& tree) {
		if (tree.empty()) return true;
		std::vector<int> sorted;
		tree.dump_sorted(std::back_inserter(sorted));
		return std::is_sorted(sorted.begin(), sorted.end(), std::less<int>());
	}

	/**
	 * Lemma 2: Structural Integrity
	 * The size reported by the tree must match the actual node count.
	 */
	size_t CountNodes(const TreeType::node* n) {
		if (!n) return 0;
		return 1 + CountNodes(n->left) + CountNodes(n->right);
	}
};

/**
 * Theorem: Insertion Splay Invariant
 * Post-condition: insert(v) => root->data == v.
 */
TEST_F(SplayTreeTest, InsertionSplayInvariant) {
	TreeType tree;
	std::vector<int> values = {50, 20, 70, 10, 30, 60, 80};

	for (int v : values) {
		tree.insert(v);
		ASSERT_NE(tree.root(), nullptr);
		ASSERT_EQ(tree.root()->data, v) << "Failure: Node " << v << " was not moved to root after insertion.";
		ASSERT_TRUE(VerifyBSTInvariant(tree));
	}
}

/**
 * Theorem: Access Splay Invariant
 * Post-condition: contains(v) == true => root->data == v.
 * Post-condition: contains(v) == false => root->data == [last accessed node].
 */
TEST_F(SplayTreeTest, AccessSplayInvariant) {
	TreeType tree;
	for (int i = 0; i < 100; i += 10) tree.insert(i);

	// Case 1: Successful search
	ASSERT_TRUE(tree.contains(30));
	ASSERT_EQ(tree.root()->data, 30);

	// Case 2: Unsuccessful search (search for 35 hits 30 or 40)
	tree.contains(35);
	int root_val = tree.root()->data;
	ASSERT_TRUE(root_val == 30 || root_val == 40);
	ASSERT_TRUE(VerifyBSTInvariant(tree));
}

/**
 * Theorem: Deletion and Join Logic
 * Verifies that remove(v) results in v \notin T and maintains BST order.
 */
TEST_F(SplayTreeTest, DeletionLogic) {
	TreeType tree;
	std::vector<int> values = {10, 20, 30, 40, 50};
	for (int v : values) tree.insert(v);

	// Remove leaf-equivalent (after splay)
	tree.remove(50);
	ASSERT_FALSE(tree.contains(50));
	ASSERT_EQ(tree.size(), 4);

	// Remove internal node
	tree.remove(20);
	ASSERT_FALSE(tree.contains(20));
	ASSERT_TRUE(VerifyBSTInvariant(tree));

	// Remove root until empty
	tree.remove(10);
	tree.remove(30);
	tree.remove(40);
	ASSERT_TRUE(tree.empty());
	ASSERT_EQ(tree.root(), nullptr);
}

/**
 * Theorem: Heavy Stress and Set Equivalence
 * Stochastic verification against std::set over N operations.
 */
TEST_F(SplayTreeTest, StochasticStressTest) {
	TreeType tree;
	std::set<int> canonical;
	const int N = 20000;
	std::uniform_int_distribution<int> dist(-10000, 10000);
	std::uniform_int_distribution<int> op_dist(0, 2);

	for (int i = 0; i < N; ++i) {
		int val = dist(gen);
		int op = op_dist(gen);

		if (op == 0) { // Insert
			tree.insert(val);
			canonical.insert(val);
		} else if (op == 1) { // Contains
			ASSERT_EQ(tree.contains(val), canonical.count(val) > 0);
		} else { // Remove
			tree.remove(val);
			canonical.erase(val);
		}

		if (i % 500 == 0) {
			ASSERT_EQ(tree.size(), canonical.size());
			ASSERT_TRUE(VerifyBSTInvariant(tree));
			ASSERT_EQ(CountNodes(tree.root()), tree.size());
		}
	}
}

/**
 * Theorem: Zig-Zig Amortization (Path Halving)
 * A sequential insertion followed by a splay of the deepest node
 * must reduce the Internal Path Length (IPL).
 */
TEST_F(SplayTreeTest, PathHalvingHeuristic) {
	TreeType tree;
	const int N = 1000;
	// Force a degenerate linear tree (right-skewed)
	for (int i = 0; i < N; ++i) {
		tree.insert(i);
	}

	size_t ipl_before = tree.internal_path_length();

	// Splay the minimum element (0) which is at depth N-1
	tree.contains(0);

	size_t ipl_after = tree.internal_path_length();

	// Heuristic: after splaying the deepest node, the IPL should be less than half of the original
	size_t ipl_skewed_threshold = ipl_before * 0.8;
	ASSERT_LT(ipl_after, ipl_skewed_threshold)
		<< "Splay did not sufficiently rebalance the skewed tree. IPL before: "
		<< ipl_before << ", after: " << ipl_after;
	ASSERT_TRUE(VerifyBSTInvariant(tree));
}

/**
 * Theorem: Boundary and Edge Cases
 */
TEST_F(SplayTreeTest, EdgeCases) {
	TreeType tree;

	// Empty tree operations
	ASSERT_FALSE(tree.contains(42));
	tree.remove(42);
	ASSERT_EQ(tree.size(), 0);

	// Duplicate insertions
	tree.insert(10);
	tree.insert(10);
	ASSERT_EQ(tree.size(), 1) << "Size incremented on duplicate insertion.";

	// Large values
	tree.insert(std::numeric_limits<int>::max());
	tree.insert(std::numeric_limits<int>::min());
	ASSERT_TRUE(tree.contains(std::numeric_limits<int>::max()));
	ASSERT_TRUE(VerifyBSTInvariant(tree));
}




//  ____                                _                                         _           _
// |  _ \  ___  ___ _ __  ___  ___  ___| | __      __ _  ___ _ __   ___ _ __ __ _| |_ ___  __| |
// | | | |/ _ \/ _ \ '_ \/ __|/ _ \/ _ \ |/ /____ / _` |/ _ \ '_ \ / _ \ '__/ _` | __/ _ \/ _` |
// | |_| |  __/  __/ |_) \__ \  __/  __/   <_____| (_| |  __/ | | |  __/ | | (_| | ||  __/ (_| |
// |____/ \___|\___| .__/|___/\___|\___|_|\_\     \__, |\___|_| |_|\___|_|  \__,_|\__\___|\__,_|
//                 |_|                            |___/

/**
 * Helper to verify BST property for any comparator.
 */
template <typename Tree>
bool verify_bst(const Tree& tree) {
	if (tree.empty()) return true;
	std::vector<typename Tree::value_type> sorted;
	tree.dump_sorted(std::back_inserter(sorted));
	return std::is_sorted(sorted.begin(), sorted.end(), tree.less_than);
}

/**
 * Helper to count nodes in a subtree (structural integrity).
 */
template <typename Node>
size_t count_nodes(const Node* n) {
	if (!n) return 0;
	return 1 + count_nodes(n->left) + count_nodes(n->right);
}

/**
 * Fixture for int splay tree.
 */
class SplayTreeIntTest : public ::testing::Test {
protected:
	using Tree = splay_tree<int>;
	Tree tree;
	std::mt19937 gen{std::random_device{}()};

	void SetUp() override {
		tree.make_empty();
	}
};

/**
 * Fixture for string splay tree.
 */
class SplayTreeStringTest : public ::testing::Test {
protected:
	using Tree = splay_tree<std::string>;
	Tree tree;
	std::mt19937 gen{std::random_device{}()};
};

/**
 * Fixture for custom comparator (greater).
 */
class SplayTreeGreaterTest : public ::testing::Test {
protected:
	using Tree = splay_tree<int, std::greater<int>>;
	Tree tree;
	std::mt19937 gen{std::random_device{}()};
};

// ---------------------------------------------------------------------------
// 1. Basic invariants after insertion
// ---------------------------------------------------------------------------
TEST_F(SplayTreeIntTest, InsertionSplaysToRoot) {
	std::vector<int> values = {50, 20, 70, 10, 30, 60, 80};
	std::set <int> ss;
	for (int v : values) {
		std::cout << "T: " << tree << std::endl;
		tree.insert(v);
		ss.insert (v);
		ASSERT_NE(tree.root(), nullptr);
		ASSERT_EQ(tree.root()->data, v);
		ASSERT_TRUE(verify_bst(tree));
		ASSERT_EQ(tree.size(), ss.size ());
	}
}

TEST_F(SplayTreeIntTest, DuplicateInsertions) {
	tree.insert(42);
	tree.insert(42);
	ASSERT_EQ(tree.size(), 1);
	ASSERT_EQ(tree.root()->data, 42);
	ASSERT_TRUE(verify_bst(tree));
}

// ---------------------------------------------------------------------------
// 2. Contains and splay behaviour
// ---------------------------------------------------------------------------
TEST_F(SplayTreeIntTest, ContainsSplaysFoundElement) {
	std::vector<int> values = {10, 20, 30, 40, 50};
	for (int v : values) tree.insert(v);

	// Successful search
	for (int v : values) {
		ASSERT_TRUE(tree.contains(v));
		ASSERT_EQ(tree.root()->data, v);
		ASSERT_TRUE(verify_bst(tree));
	}
}

TEST_F(SplayTreeIntTest, ContainsSplaysNearestOnMiss) {
	for (int i = 0; i < 100; i += 10) tree.insert(i);

	tree.contains(35);
	int root_val = tree.root()->data;
	ASSERT_TRUE(root_val == 30 || root_val == 40);
	ASSERT_TRUE(verify_bst(tree));
}

TEST_F(SplayTreeIntTest, ContainsOnEmpty) {
	ASSERT_FALSE(tree.contains(0));
	ASSERT_EQ(tree.root(), nullptr);
}

// ---------------------------------------------------------------------------
// 3. Deletion
// ---------------------------------------------------------------------------
TEST_F(SplayTreeIntTest, DeleteLeaf) {
	tree.insert(5);
	tree.insert(3);
	tree.insert(7);
	tree.remove(3);
	ASSERT_FALSE(tree.contains(3));
	ASSERT_EQ(tree.size(), 2);
	ASSERT_TRUE(verify_bst(tree));
	// After deletion, the parent (5) should be splayed if the stack is not empty
	ASSERT_EQ(tree.root()->data, 5);
}

TEST_F(SplayTreeIntTest, DeleteRoot) {
	tree.insert(42);
	tree.remove(42);
	ASSERT_TRUE(tree.empty());
	ASSERT_EQ(tree.root(), nullptr);
}

TEST_F(SplayTreeIntTest, DeleteMissingKey) {
	for (int i = 0; i < 10; ++i) tree.insert(i);
	tree.remove(100);
	ASSERT_EQ(tree.size(), 10);
	ASSERT_TRUE(verify_bst(tree));
	// The parent of the missing location should be splayed
	int root_val = tree.root()->data;
	ASSERT_TRUE(root_val == 9 || root_val == 0); // depending on comparator
}

TEST_F(SplayTreeIntTest, DeleteAllElements) {
	const int N = 1000;
	for (int i = 0; i < N; ++i) tree.insert(i);
	for (int i = 0; i < N; ++i) tree.remove(i);
	ASSERT_TRUE(tree.empty());
	ASSERT_EQ(tree.root(), nullptr);
}

// ---------------------------------------------------------------------------
// 4. Random stress test against std::set (up to 200k operations)
// ---------------------------------------------------------------------------
TEST_F(SplayTreeIntTest, StressRandomVsSet) {
	const int OPERATIONS = 200000;
	std::uniform_int_distribution<int> val_dist(-10000, 10000);
	std::uniform_int_distribution<int> op_dist(0, 2);
	std::set<int> ref;
	int inserts = 0, removals = 0, contains = 0;

	for (int i = 0; i < OPERATIONS; ++i) {
		int v = val_dist(gen);
		int op = op_dist(gen);

		if (op == 0) { // insert
			tree.insert(v);
			ref.insert(v);
			++inserts;
		} else if (op == 1) { // contains
			bool found = tree.contains(v);
			ASSERT_EQ(found, ref.count(v) > 0);
			++contains;
		} else { // remove
			tree.remove(v);
			ref.erase(v);
			++removals;
		}

		if (i % 5000 == 0) {
			// Periodic invariant checks (avoid slowing down the test too much)
			ASSERT_EQ(tree.size(), ref.size());
			ASSERT_TRUE(verify_bst(tree));
			ASSERT_EQ(count_nodes(tree.root()), tree.size());
		}
	}
	std::cout << "Stress test completed: " << inserts << " inserts, "
			  << removals << " removals, " << contains << " contains.\n";
}

// ---------------------------------------------------------------------------
// 5. Large tree insertion and rebalancing (100k nodes)
// ---------------------------------------------------------------------------
TEST_F(SplayTreeIntTest, LargeInsertionAndSplay) {
	const int N = 100000;
	std::vector<int> values(N);
	std::iota(values.begin(), values.end(), 0);
	std::shuffle(values.begin(), values.end(), gen);

	for (int v : values) {
		tree.insert(v);
	}

	ASSERT_EQ(tree.size(), N);
	ASSERT_TRUE(verify_bst(tree));
	ASSERT_EQ(count_nodes(tree.root()), N);

	// Access all nodes in random order to verify splay
	std::shuffle(values.begin(), values.end(), gen);
	for (int v : values) {
		ASSERT_TRUE(tree.contains(v));
		ASSERT_EQ(tree.root()->data, v);
	}
}

// ---------------------------------------------------------------------------
// 6. Internal path length reduction (splay rebalancing)
// ---------------------------------------------------------------------------
TEST_F(SplayTreeIntTest, SplayReducesPathLength) {
	const int N = 1000;
	// Build a degenerate right‑skewed tree by inserting in ascending order
	for (int i = 0; i < N; ++i) tree.insert(i);

	size_t ipl_before = tree.internal_path_length();
	// Splay the deepest node (the minimum, which is at depth N-1)
	tree.contains(0);
	size_t ipl_after = tree.internal_path_length();

	// After splaying, the internal path length must be strictly less
	ASSERT_LT(ipl_after, ipl_before);
	ASSERT_TRUE(verify_bst(tree));
}

// ---------------------------------------------------------------------------
// 7. Edge cases: int min/max, large values, negative
// ---------------------------------------------------------------------------
TEST_F(SplayTreeIntTest, MinMaxValues) {
	int min = std::numeric_limits<int>::min();
	int max = std::numeric_limits<int>::max();

	tree.insert(min);
	tree.insert(max);
	tree.insert(0);

	ASSERT_TRUE(tree.contains(min));
	ASSERT_TRUE(tree.contains(max));
	ASSERT_TRUE(tree.contains(0));
	ASSERT_EQ(tree.size(), 3);
	ASSERT_TRUE(verify_bst(tree));

	tree.remove(max);
	ASSERT_FALSE(tree.contains(max));
	ASSERT_EQ(tree.size(), 2);
	ASSERT_TRUE(verify_bst(tree));
}

// ---------------------------------------------------------------------------
// 8. Non‑trivial types: string
// ---------------------------------------------------------------------------
TEST_F(SplayTreeStringTest, StringOperations) {
	std::vector<std::string> words = {"apple", "zebra", "banana", "cherry", "date"};
	for (const auto& w : words) tree.insert(w);
	ASSERT_EQ(tree.size(), words.size());
	ASSERT_TRUE(verify_bst(tree));

	for (const auto& w : words) {
		ASSERT_TRUE(tree.contains(w));
		ASSERT_EQ(tree.root()->data, w);
	}

	tree.remove("banana");
	ASSERT_FALSE(tree.contains("banana"));
	ASSERT_EQ(tree.size(), words.size() - 1);
	ASSERT_TRUE(verify_bst(tree));
}

// ---------------------------------------------------------------------------
// 9. Custom comparator (std::greater)
// ---------------------------------------------------------------------------
TEST_F(SplayTreeGreaterTest, SortedOrder) {
	std::vector<int> values = {5, 1, 9, 3, 7};
	for (int v : values) tree.insert(v);

	std::vector<int> dumped;
	tree.dump_sorted(std::back_inserter(dumped));
	std::vector<int> expected = {9, 7, 5, 3, 1};
	ASSERT_EQ(dumped, expected);
	ASSERT_TRUE(verify_bst(tree));
}

TEST_F(SplayTreeGreaterTest, ContainsAfterSplay) {
	for (int i = 0; i < 10; ++i) tree.insert(i);
	tree.contains(5);
	// With greater comparator, the largest element in left subtree? Not needed; just check BST.
	ASSERT_TRUE(verify_bst(tree));
}

// ---------------------------------------------------------------------------
// 10. Copy, move, assignment (if the tree supports these)
// ---------------------------------------------------------------------------
TEST_F(SplayTreeIntTest, CopyConstructor) {
	for (int i = 0; i < 100; ++i) tree.insert(i);
	splay_tree<int> copy(tree);
	ASSERT_EQ(copy.size(), tree.size());
	ASSERT_TRUE(verify_bst(copy));
	// Verify that the root is different (deep copy)
	if (!tree.empty() && !copy.empty())
		ASSERT_NE(tree.root(), copy.root());
	// Compare sorted output
	std::vector<int> orig, copied;
	tree.dump_sorted(std::back_inserter(orig));
	copy.dump_sorted(std::back_inserter(copied));
	ASSERT_EQ(orig, copied);
}

TEST_F(SplayTreeIntTest, MoveConstructor) {
	for (int i = 0; i < 100; ++i) tree.insert(i);
	size_t old_size = tree.size();
	splay_tree<int> moved(std::move(tree));
	ASSERT_EQ(moved.size(), old_size);
	ASSERT_TRUE(tree.empty());
	ASSERT_TRUE(verify_bst(moved));
}

TEST_F(SplayTreeIntTest, AssignmentOperator) {
	splay_tree<int> other;
	for (int i = 0; i < 100; ++i) other.insert(i);
	tree = other;
	ASSERT_EQ(tree.size(), other.size());
	ASSERT_TRUE(verify_bst(tree));
	// Ensure deep copy
	if (!tree.empty() && !other.empty())
		ASSERT_NE(tree.root(), other.root());
}

// ---------------------------------------------------------------------------
// 11. Clearing the tree
// ---------------------------------------------------------------------------
TEST_F(SplayTreeIntTest, Clear) {
	for (int i = 0; i < 1000; ++i) tree.insert(i);
	tree.make_empty();
	ASSERT_TRUE(tree.empty());
	ASSERT_EQ(tree.root(), nullptr);
	ASSERT_EQ(tree.size(), 0);
	ASSERT_FALSE(tree.contains(0));
}

// ---------------------------------------------------------------------------
// 12. Consistency after many splays (random access pattern)
// ---------------------------------------------------------------------------
TEST_F(SplayTreeIntTest, RandomAccessSplay) {
	const int N = 10000;
	std::vector<int> values(N);
	std::iota(values.begin(), values.end(), 0);
	std::shuffle(values.begin(), values.end(), gen);

	for (int v : values) tree.insert(v);

	// Randomly access elements
	std::shuffle(values.begin(), values.end(), gen);
	for (int v : values) {
		ASSERT_TRUE(tree.contains(v));
		ASSERT_EQ(tree.root()->data, v);
		ASSERT_TRUE(verify_bst(tree));
	}
}

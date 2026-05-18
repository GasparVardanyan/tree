//   ____                _       _                                       _           _
//  / ___| ___ _ __ ___ (_)_ __ (_)       __ _  ___ _ __   ___ _ __ __ _| |_ ___  __| |
// | |  _ / _ \ '_ ` _ \| | '_ \| |_____ / _` |/ _ \ '_ \ / _ \ '__/ _` | __/ _ \/ _` |
// | |_| |  __/ | | | | | | | | | |_____| (_| |  __/ | | |  __/ | | (_| | ||  __/ (_| |
//  \____|\___|_| |_| |_|_|_| |_|_|      \__, |\___|_| |_|\___|_|  \__,_|\__\___|\__,_|
//                                       |___/

# include <gtest/gtest.h>
# include <algorithm>
# include <vector>
# include <string>

# include <tree/binary_search_tree.hpp>
using namespace tree;

class BinarySearchTreeTest : public ::testing::Test {
protected:
	binary_search_tree<int> bst;

	// Helper to verify BST invariant: Inorder traversal must be sorted
	void VerifyInvariant(const binary_search_tree<int>& tree) {
		if (tree.empty()) return;
		vector<int> v = static_cast<vector<int>>(tree);
		EXPECT_TRUE(std::is_sorted(v.begin(), v.end()));
	}

	// Creates a tree:
	//      50
	//     /  \
	//    30   70
	//   /  \    \
	//  20  40   80
	void PopulateStandard(binary_search_tree<int>& tree) {
		tree.insert(50);
		tree.insert(30);
		tree.insert(70);
		tree.insert(20);
		tree.insert(40);
		tree.insert(80);
	}
};

// --- Core Functionality ---

TEST_F(BinarySearchTreeTest, InsertionAndSize) {
	EXPECT_TRUE(bst.empty());
	bst.insert(10);
	bst.insert(20);
	bst.insert(5);

	EXPECT_EQ(bst.size(), 3);
	EXPECT_FALSE(bst.empty());

	// Test duplicate insertion (should be ignored)
	bst.insert(10);
	EXPECT_EQ(bst.size(), 3);
	VerifyInvariant(bst);
}

TEST_F(BinarySearchTreeTest, SearchAndExtremes) {
	PopulateStandard(bst);

	EXPECT_TRUE(bst.contains(40));
	EXPECT_TRUE(bst.contains(80));
	EXPECT_FALSE(bst.contains(99));

	EXPECT_EQ(bst.find_min()->data, 20);
	EXPECT_EQ(bst.find_max()->data, 80);

	EXPECT_TRUE(bst.contains(20));
}

// --- Deletion Logic ---

TEST_F(BinarySearchTreeTest, RemoveLeafNode) {
	PopulateStandard(bst);
	bst.remove(20); // Leaf
	EXPECT_EQ(bst.size(), 5);
	EXPECT_FALSE(bst.contains(20));
	VerifyInvariant(bst);
}

TEST_F(BinarySearchTreeTest, RemoveNodeWithOneChild) {
	PopulateStandard(bst);
	bst.remove(70); // Has one child (80)
	EXPECT_EQ(bst.size(), 5);
	EXPECT_TRUE(bst.contains(80));
	VerifyInvariant(bst);
}

TEST_F(BinarySearchTreeTest, RemoveNodeWithTwoChildren) {
	PopulateStandard(bst);
	// This tests your specific logic: grafting left subtree to leftmost of right
	bst.remove(30);
	EXPECT_EQ(bst.size(), 5);
	EXPECT_FALSE(bst.contains(30));
	EXPECT_TRUE(bst.contains(20));
	EXPECT_TRUE(bst.contains(40));
	VerifyInvariant(bst);
}

TEST_F(BinarySearchTreeTest, RemoveRoot) {
	bst.insert(10);
	bst.insert(5);
	bst.insert(15);
	bst.remove(10);

	EXPECT_EQ(bst.size(), 2);

	VerifyInvariant(bst);
}

// --- Lifecycle (Rule of Five) ---

TEST_F(BinarySearchTreeTest, CopyAndMove) {
	PopulateStandard(bst);

	// Copy Constructor
	binary_search_tree<int> copy(bst);
	EXPECT_EQ(copy.size(), bst.size());
	EXPECT_NE(copy.root(), bst.root()); // Deep copy check

	// Move Assignment
	binary_search_tree<int> moved;
	moved = std::move(bst);
	EXPECT_EQ(moved.size(), 6);
	EXPECT_TRUE(bst.empty());
	EXPECT_EQ(bst.root(), nullptr);
}

// --- Advanced/Strict Tests ---

TEST_F(BinarySearchTreeTest, CustomComparatorDescending) {
	// Testing the strict weak ordering requirement with std::greater
	binary_search_tree<int, std::greater <int>> desc_bst;
	desc_bst.insert(10);
	desc_bst.insert(20);
	desc_bst.insert(5);

	// Inorder should now be descending
	std::vector<int> v;
	desc_bst.dump_sorted(std::back_inserter(v));

	std::vector<int> expected = {20, 10, 5};
	EXPECT_EQ(v, expected);
	EXPECT_EQ(desc_bst.find_min()->data, 20); // Min in terms of comparator
}

TEST_F(BinarySearchTreeTest, VectorConversion) {
	PopulateStandard(bst);
	vector<int> v = static_cast<vector<int>>(bst);

	std::vector<int> expected = {20, 30, 40, 50, 70, 80};
	EXPECT_EQ(std::vector <int> (v.cbegin (), v.cend ()), expected);
}

TEST_F(BinarySearchTreeTest, MakeEmpty) {
	PopulateStandard(bst);
	bst.make_empty();
	EXPECT_EQ(bst.size(), 0);
	EXPECT_EQ(bst.root(), nullptr);
	EXPECT_TRUE(bst.empty());
}

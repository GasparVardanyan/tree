//   ____                _       _                                       _           _
//  / ___| ___ _ __ ___ (_)_ __ (_)       __ _  ___ _ __   ___ _ __ __ _| |_ ___  __| |
// | |  _ / _ \ '_ ` _ \| | '_ \| |_____ / _` |/ _ \ '_ \ / _ \ '__/ _` | __/ _ \/ _` |
// | |_| |  __/ | | | | | | | | | |_____| (_| |  __/ | | |  __/ | | (_| | ||  __/ (_| |
//  \____|\___|_| |_| |_|_|_| |_|_|      \__, |\___|_| |_|\___|_|  \__,_|\__\___|\__,_|
//                                       |___/

# include <gtest/gtest.h>
# include <vector>
# include <string>
# include <sstream>

# include <tree/binary_tree_node.hpp>
using namespace tree;

class BinaryTreeNodeTest : public ::testing::Test {
protected:
	// Helper to build a complex, asymmetrical tree:
	//              10
	//           /      \
	//         5         15
	//       /   \      /   \
	//      2     7    12    20
	//     / \     \        /
	//    1   3     8      18
	binary_tree_node<int>* BuildComplexTree() {
		auto* n1 = new binary_tree_node<int>(1);
		auto* n3 = new binary_tree_node<int>(3);
		auto* n2 = new binary_tree_node<int>(2, n1, n3);

		auto* n8 = new binary_tree_node<int>(8);
		auto* n7 = new binary_tree_node<int>(7, nullptr, n8);
		auto* n5 = new binary_tree_node<int>(5, n2, n7);

		auto* n12 = new binary_tree_node<int>(12);
		auto* n18 = new binary_tree_node<int>(18);
		auto* n20 = new binary_tree_node<int>(20, n18, nullptr);
		auto* n15 = new binary_tree_node<int>(15, n12, n20);

		return new binary_tree_node<int>(10, n5, n15);
	}
};

// --- Lifecycle & Rule of Five (Fixed to use TEST_F) ---

TEST_F(BinaryTreeNodeTest, ConstructorAndDestructor) {
	auto* root = new binary_tree_node<int>(1);
	root->left = new binary_tree_node<int>(2);
	EXPECT_EQ(root->left->data, 2);
	delete root; // Iterative cleanup test
}

TEST_F(BinaryTreeNodeTest, CopyConstructorDeepValidation) {
	auto* original = BuildComplexTree();
	binary_tree_node<int> copy(*original);

	// Verify values match but addresses differ
	EXPECT_EQ(copy.data, 10);
	EXPECT_NE(copy.left, original->left);
	EXPECT_EQ(copy.left->right->right->data, 8); // Deep node check

	delete original;
	EXPECT_EQ(copy.left->right->right->data, 8); // Verify independence
}

TEST_F(BinaryTreeNodeTest, MoveAssignmentSelfAssignment) {
	auto* root = BuildComplexTree();

	// Testing move to self (should be handled by "if (this != &other)")
	*root = std::move(*root);

	EXPECT_EQ(root->data, 10);
	EXPECT_NE(root->left, nullptr);
	delete root;
}

// --- Traversal Accuracy Tests ---

TEST_F(BinaryTreeNodeTest, ComplexInorderTraversal) {
	auto* root = BuildComplexTree();
	std::vector<int> result;
	root->inorder_traverse([&](const auto* n, const auto*) {
		result.push_back(n->data);
	});

	// Expected Inorder: 1, 2, 3, 5, 7, 8, 10, 12, 15, 18, 20
	std::vector<int> expected = {1, 2, 3, 5, 7, 8, 10, 12, 15, 18, 20};
	EXPECT_EQ(result, expected);
	delete root;
}

TEST_F(BinaryTreeNodeTest, ParentPointerVerification) {
	auto* root = BuildComplexTree();

	// During preorder, verify that the 'parent' argument is actually the parent
	root->preorder_traverse([&](const auto* n, const auto* p) {
		if (p != nullptr) {
			bool is_child = (p->left == n || p->right == n);
			EXPECT_TRUE(is_child) << "Node " << n->data << " claims "
								  << p->data << " is parent, but no link found.";
		}
	});
	delete root;
}

TEST_F(BinaryTreeNodeTest, LevelOrderMultiLevelVerification) {
	auto* root = BuildComplexTree();
	std::vector<std::vector<int>> levels(4);

	root->level_order_traverse([&](const auto* n, const auto*, size_t level) {
		levels[level].push_back(n->data);
	});

	EXPECT_EQ(levels[0], std::vector<int>({10}));
	EXPECT_EQ(levels[1], std::vector<int>({5, 15}));
	EXPECT_EQ(levels[2], std::vector<int>({2, 7, 12, 20}));
	EXPECT_EQ(levels[3], std::vector<int>({1, 3, 8, 18}));

	delete root;
}

// --- Edge Cases ---

TEST_F(BinaryTreeNodeTest, DeepStackSafety) {
	// Create a very deep skewed tree (5000 nodes)
	auto* root = new binary_tree_node<int>(0);
	auto* curr = root;
	for(int i = 1; i < 5000; ++i) {
		curr->left = new binary_tree_node<int>(i);
		curr = curr->left;
	}

	// If destructor is recursive, this would likely SegFault/Stack Overflow
	// Since yours is iterative (queue-based), this will pass.
	delete root;
	SUCCEED();
}

TEST_F(BinaryTreeNodeTest, StreamFormattingComplex) {
	auto* root = BuildComplexTree();
	std::stringstream ss;
	ss << *root;
	std::string out = ss.str();

	// Check for specific structural markers in your stream operator logic
	EXPECT_TRUE(out.find("10") != std::string::npos);
	EXPECT_TRUE(out.find("(L)") != std::string::npos);
	EXPECT_TRUE(out.find("(R)") != std::string::npos);

	delete root;
}

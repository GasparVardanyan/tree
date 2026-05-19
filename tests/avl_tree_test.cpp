# include <gtest/gtest.h>
# include <cstddef>
# include <algorithm>
# include <chrono>
# include <cmath>
# include <iomanip>
# include <iostream>
# include <limits>
# include <numeric>
# include <ostream>
# include <random>
# include <ratio>
# include <vector>
# include <initializer_list>
# include <string>

# include <tree/avl_tree.hpp>
using namespace tree;

inline const std::size_t nIter = 100;

template <typename T>
static bool verify_avl_balance (const avl_tree <T> & tree) {
	if (0 == tree.size ()) {
		return true;
	}

	stack <const typename avl_tree <T>::node *> node_stack;
	node_stack.push (tree.root ());

	bool ok = true;

	while (false == node_stack.empty ()) {
		auto n = node_stack.top ();
		node_stack.pop ();

		std::size_t lh, rh;

		if (nullptr != n->left) {
			lh = n->left->data.height_plus_one;
			node_stack.push (n->left);
		}
		else {
			lh = 0;
		}

		if (nullptr != n->right) {
			rh = n->right->data.height_plus_one;
			node_stack.push (n->right);
		}
		else {
			rh = 0;
		}

		if (lh > rh) {
			if (n->data.height_plus_one != lh + 1) {
				ok = false;
				break;
			}
			if (lh - rh > 1) {
				ok = false;
				break;
			}
		}
		else if (rh > lh) {
			if (n->data.height_plus_one != rh + 1) {
				ok = false;
				break;
			}
			if (rh - lh > 1) {
				ok = false;
				break;
			}
		}
		else {
			if (n->data.height_plus_one != lh + 1) {
				ok = false;
				break;
			}
		}
	}

	return ok;
}

template <typename T>
static bool verify_avl_order (const avl_tree <T> & tree) {
	bool ok = true;

	vector <typename avl_tree <T>::value_type> v = tree;

	for (std::size_t i = 1; i < v.size (); i++) {
		if (false == tree.less_than (v [i - 1], v [i])) {
			ok = false;
			break;
		}
	}

	return ok;
}

template <typename T = int>
static std::vector <T> random_permutation (std::size_t n, std::mt19937 & rng) {
	std::vector <T> values (n);
	std::iota (values.begin (), values.end (), 0);
	std::ranges::shuffle (values, rng);

	return values;
}

struct TestParams {
	const std::size_t minCount = 80'000;
	const std::size_t maxCount = 100'000;
	const bool test_insert_balance = true;
	const bool test_insert_order = true;
	const bool test_remove_balance = true;
	const bool test_remove_order = true;
	const std::size_t check_interval = 0;
	const std::size_t iterations = 1;
};

template <const TestParams params>
void avl_test () {
	static_assert (0 < params.iterations && params.iterations < std::numeric_limits <std::size_t>::max (), "illegal iterations count");

	for (std::size_t i = 0; i < params.iterations; i++) {
		std::mt19937 rng (std::random_device {} ());
		std::size_t n = std::uniform_int_distribution <std::size_t> (params.minCount, params.maxCount) (rng);
		std::vector <int> values = random_permutation (n, rng);

		std::size_t counter = 0;

		avl_tree <int> avlt;

		for (int item : values) {
			avlt.insert (item);

			bool check = false;

			if constexpr (0 == params.check_interval) {
				check = true;
			}
			else if (++counter == params.check_interval) {
				counter = 0;
				check = true;
			}

			if (true == check) {
				if constexpr (true == params.test_insert_balance) {
					ASSERT_TRUE (verify_avl_balance (avlt));
				}
				if constexpr (true == params.test_insert_order) {
					ASSERT_TRUE (verify_avl_order (avlt));
				}
			}
		}

		std::cout << "Tree size: " << avlt.size () << ", height: " << (static_cast <int> (avlt.root ()->data.height_plus_one) - 1) << std::endl;

		counter = 0;

		if constexpr (true == params.test_remove_balance || true == params.test_remove_order) {
			std::shuffle (values.begin (), values.end (), rng);

			for (int item : values) {
				avlt.remove (item);

				bool check = false;

				if constexpr (0 == params.check_interval) {
					check = true;
				}
				else if (++counter == params.check_interval) {
					counter = 0;
					check = true;
				}

				if (true == check) {
					if constexpr (true == params.test_remove_balance) {
						ASSERT_TRUE (verify_avl_balance (avlt));
					}
					if constexpr (true == params.test_remove_order) {
						ASSERT_TRUE (verify_avl_order (avlt));
					}
				}
			}

			ASSERT_TRUE (avlt.empty ());
		}
	}
}

TEST(AVLSingleTreeFullInsertRemove, SmallTree)
{
	avl_test <{
		.minCount = 8'000,
		.maxCount = 10'000,
		.test_insert_balance = true,
		.test_insert_order = true,
		.test_remove_balance = true,
		.test_remove_order = true,
		.check_interval = 0,
		.iterations = 1
	}> ();
}

TEST(AVLSingleTreeFullInsertRemove, BigTreeSmallIntervals)
{
	avl_test <{
		.minCount = 80'000,
		.maxCount = 100'000,
		.test_insert_balance = true,
		.test_insert_order = true,
		.test_remove_balance = true,
		.test_remove_order = true,
		.check_interval = 100,
		.iterations = 1
	}> ();
}

TEST(AVLSingleTreeFullInsertRemove, Balance)
{
	avl_test <{
		.minCount = 80'000,
		.maxCount = 100'000,
		.test_insert_balance = true,
		.test_insert_order = false,
		.test_remove_balance = true,
		.test_remove_order = false,
		.check_interval = 0,
		.iterations = 1
	}> ();
}

TEST(AVLSingleTreeFullInsertRemove, Order)
{
	avl_test <{
		.minCount = 80'000,
		.maxCount = 100'000,
		.test_insert_balance = false,
		.test_insert_order = true,
		.test_remove_balance = false,
		.test_remove_order = true,
		.check_interval = 0,
		.iterations = 1
	}> ();
}

TEST(AVLBruteForceInsert, Balance)
{
	avl_test <{
		.minCount = 8'000,
		.maxCount = 10'000,
		.test_insert_balance = true,
		.test_insert_order = false,
		.test_remove_balance = false,
		.test_remove_order = false,
		.check_interval = 0,
		.iterations = nIter
	}> ();
}

TEST(AVLBruteForceInsert, Order)
{
	avl_test <{
		.minCount = 8'000,
		.maxCount = 10'000,
		.test_insert_balance = false,
		.test_insert_order = true,
		.test_remove_balance = false,
		.test_remove_order = false,
		.check_interval = 0,
		.iterations = nIter
	}> ();
}

TEST(AVLBruteForceRemove, Balance)
{
	avl_test <{
		.minCount = 8'000,
		.maxCount = 10'000,
		.test_insert_balance = false,
		.test_insert_order = false,
		.test_remove_balance = true,
		.test_remove_order = false,
		.check_interval = 0,
		.iterations = nIter
	}> ();
}

TEST(AVLBruteForceRemove, Order)
{
	avl_test <{
		.minCount = 8'000,
		.maxCount = 10'000,
		.test_insert_balance = false,
		.test_insert_order = false,
		.test_remove_balance = false,
		.test_remove_order = true,
		.check_interval = 0,
		.iterations = nIter
	}> ();
}

TEST(AVLBigTreeInsert, Balance)
{
	avl_test <{
		.minCount = 80'000,
		.maxCount = 100'000,
		.test_insert_balance = true,
		.test_insert_order = false,
		.test_remove_balance = false,
		.test_remove_order = false,
		.check_interval = 0,
		.iterations = 1
	}> ();
}

TEST(AVLBigTreeInsert, Order)
{
	avl_test <{
		.minCount = 80'000,
		.maxCount = 100'000,
		.test_insert_balance = false,
		.test_insert_order = true,
		.test_remove_balance = false,
		.test_remove_order = false,
		.check_interval = 0,
		.iterations = 1
	}> ();
}

TEST(AVLBigTreeRemove, Balance)
{
	avl_test <{
		.minCount = 80'000,
		.maxCount = 100'000,
		.test_insert_balance = false,
		.test_insert_order = false,
		.test_remove_balance = true,
		.test_remove_order = false,
		.check_interval = 0,
		.iterations = 1
	}> ();
}

TEST(AVLBigTreeRemove, Order)
{
	avl_test <{
		.minCount = 80'000,
		.maxCount = 100'000,
		.test_insert_balance = false,
		.test_insert_order = false,
		.test_remove_balance = false,
		.test_remove_order = true,
		.check_interval = 0,
		.iterations = 1
	}> ();
}



//   ____                _       _                                       _           _
//  / ___| ___ _ __ ___ (_)_ __ (_)       __ _  ___ _ __   ___ _ __ __ _| |_ ___  __| |_
// | |  _ / _ \ '_ ` _ \| | '_ \| |_____ / _` |/ _ \ '_ \ / _ \ '__/ _` | __/ _ \/ _` (_)
// | |_| |  __/ | | | | | | | | | |_____| (_| |  __/ | | |  __/ | | (_| | ||  __/ (_| |_
//  \____|\___|_| |_| |_|_|_| |_|_|      \__, |\___|_| |_|\___|_|  \__,_|\__\___|\__,_(_)
//                                       |___/

TEST(AVLManualCornerCases, EmptyAndSingle) {
	avl_tree<int> tree;
	ASSERT_TRUE(tree.empty());

	// Single node
	tree.insert(10);
	ASSERT_EQ(tree.size(), 1);
	ASSERT_TRUE(verify_avl_balance(tree));

	// Remove only node
	tree.remove(10);
	ASSERT_TRUE(tree.empty());
	ASSERT_EQ(tree.size(), 0);
}

TEST(AVLManualCornerCases, DuplicateHandling) {
	avl_tree<int> tree;
	tree.insert(10);
	tree.insert(10); // BST logic usually ignores or overwrites
	ASSERT_EQ(tree.size(), 1);
	ASSERT_TRUE(verify_avl_balance(tree));
}

TEST(AVLManualCornerCases, InsertionRotations) {
	// LL Rotation (Left-Left)
	{
		avl_tree<int> tree;
		for (int i : {30, 20, 10}) tree.insert(i);
		ASSERT_TRUE(verify_avl_balance(tree));
		ASSERT_EQ(tree.root()->data.value, 20);
	}
	// RR Rotation (Right-Right)
	{
		avl_tree<int> tree;
		for (int i : {10, 20, 30}) tree.insert(i);
		ASSERT_TRUE(verify_avl_balance(tree));
		ASSERT_EQ(tree.root()->data.value, 20);
	}
	// LR Rotation (Left-Right)
	{
		avl_tree<int> tree;
		for (int i : {30, 10, 20}) tree.insert(i);
		ASSERT_TRUE(verify_avl_balance(tree));
		ASSERT_EQ(tree.root()->data.value, 20);
	}
	// RL Rotation (Right-Left)
	{
		avl_tree<int> tree;
		for (int i : {10, 30, 20}) tree.insert(i);
		ASSERT_TRUE(verify_avl_balance(tree));
		ASSERT_EQ(tree.root()->data.value, 20);
	}
}

TEST(AVLManualCornerCases, RemovalScenarios) {
	// Remove leaf
	{
		avl_tree<int> tree;
		for (int i : {20, 10, 30}) tree.insert(i);
		tree.remove(10);
		ASSERT_EQ(tree.size(), 2);
		ASSERT_TRUE(verify_avl_balance(tree));
	}
	// Remove node with 1 child
	{
		avl_tree<int> tree;
		for (int i : {20, 10, 30, 5}) tree.insert(i);
		tree.remove(10);
		ASSERT_TRUE(verify_avl_balance(tree));
		ASSERT_TRUE(tree.contains(5));
	}
	// Remove node with 2 children (triggers predecessor/successor logic)
	{
		avl_tree<int> tree;
		for (int i : {20, 10, 30, 5, 15, 25, 35}) tree.insert(i);
		tree.remove(20); // Remove root
		ASSERT_TRUE(verify_avl_balance(tree));
		ASSERT_TRUE(verify_avl_order(tree));
	}
}

TEST(AVLManualCornerCases, RemovalPropagation) {
	avl_tree<int> tree;
	// Build a tree that requires multiple passes of rebalancing on remove
	// Sequence designed to trigger rh - lh > 1 in rebalance_after_remove
	for (int i : {50, 25, 75, 10, 30, 60, 80, 5, 15, 27, 35, 90}) tree.insert(i);

	tree.remove(5);
	ASSERT_TRUE(verify_avl_balance(tree));

	tree.remove(15);
	ASSERT_TRUE(verify_avl_balance(tree));

	// Mass removal until empty
	std::vector<int> vals = {50, 25, 75, 10, 30, 60, 80, 27, 35, 90};
	for (int v : vals) {
		tree.remove(v);
		ASSERT_TRUE(verify_avl_balance(tree));
	}
	ASSERT_TRUE(tree.empty());
}

TEST(AVLManualCornerCases, PathologicalSequences) {
	const int count = 1000;

	// Strictly Ascending
	avl_tree<int> asc;
	for (int i = 0; i < count; ++i) {
		asc.insert(i);
		if (i % 10 == 0) ASSERT_TRUE(verify_avl_balance(asc));
	}

	// Strictly Descending
	avl_tree<int> desc;
	for (int i = count; i > 0; --i) {
		desc.insert(i);
		if (i % 10 == 0) ASSERT_TRUE(verify_avl_balance(desc));
	}

	ASSERT_TRUE(verify_avl_order(asc));
	ASSERT_TRUE(verify_avl_order(desc));
}

void run_structural_test(std::size_t n) {
	avl_tree<int> tree;
	std::vector<int> data(n);
	std::iota(data.begin(), data.end(), 0);
	std::shuffle(data.begin(), data.end(), std::mt19937(42));

	auto start = std::chrono::high_resolution_clock::now();
	for (int x : data) tree.insert(x);
	auto end = std::chrono::high_resolution_clock::now();

	std::chrono::duration<double, std::micro> duration = end - start;

	// Structural Metrics
	std::size_t height = tree.root() ? tree.root()->data.height_plus_one - 1 : 0;
	std::size_t ipl = tree.internal_path_length();

	double log2n = std::log2(n);
	double height_ratio = static_cast<double>(height) / log2n;
	double avg_depth = static_cast<double>(ipl) / n;

	std::cout << "N: " << std::setw(8) << n
			  << " | Height: " << std::setw(2) << height
			  << " (Ratio: " << std::fixed << std::setprecision(2) << height_ratio << "x logN)"
			  << " | Avg Depth: " << std::setprecision(2) << avg_depth
			  << " | Time/Ins: " << std::setprecision(3) << (duration.count() / n) << "us"
			  << std::endl;
}

TEST(AVLBenchmark, ScalingAndBalance) {
	std::cout << "\n--- AVL Structural Efficiency Benchmark ---\n";
	run_structural_test (1E3);
	run_structural_test (1E4);
	run_structural_test (1E5);
	run_structural_test (1E6);
	// Sample output:
	//
	// --- AVL Structural Efficiency Benchmark ---
	// N:     1000 | Height: 11 (Ratio: 1.10x logN) | Avg Depth: 8.17  | Time/Ins: 0.465us
	// N:    10000 | Height: 15 (Ratio: 1.13x logN) | Avg Depth: 11.57 | Time/Ins: 0.399us
	// N:   100000 | Height: 19 (Ratio: 1.14x logN) | Avg Depth: 14.96 | Time/Ins: 0.524us
	// N:  1000000 | Height: 23 (Ratio: 1.15x logN) | Avg Depth: 18.31 | Time/Ins: 0.914us
}

# ifndef EXPRESSION_TREE_H
# define EXPRESSION_TREE_H

# include <concepts>
# include <functional>
# include <string>
# include <type_traits>
# include <utility>

# include <tree/config.hpp>

# include <tree/binary_tree_node.hpp>

# include "notation_converter.hpp"

struct ExpressionTree : tree::TypeConfig<>::TypeConfigStd {
protected:
	struct NotationConverter : public ::NotationConverter {
		using Token = NotationConverter::Token;
		using TokenType = NotationConverter::TokenType;
		using ::NotationConverter::Tokenize;
		using ::NotationConverter::NeedParenthesis_BalanceChecker;
		using ::NotationConverter::NeedParenthesis;
	};

	template <typename T, std::equivalence_relation <T, T> EqualTo = std::equal_to <T>>
	using binary_tree_node = tree::binary_tree_node <T, EqualTo>;

public:
	static binary_tree_node <std::string> * PostfixToTree (const std::string & s) {
		Queue <NotationConverter::Token> tokens = NotationConverter::Tokenize (s);
		Stack <binary_tree_node <std::string> *> nodes;

		while (false == tokens.empty ()) {
			NotationConverter::Token t = tokens.front ();
			tokens.pop ();

			if (NotationConverter::TokenType::Operand == t.type) {
				nodes.push (new binary_tree_node <std::string> (t.value));
			}
			else if (NotationConverter::TokenType::Operator == t.type) {
				auto o2 = nodes.top ();
				nodes.pop ();
				auto o1 = nodes.top ();
				nodes.pop ();
				nodes.push (new binary_tree_node <std::string> (t.value, o1, o2));
			}
		}

		if (1 != nodes.size ()) {
			while (false == nodes.empty ()) {
				auto n = nodes.top ();
				nodes.pop ();
				delete n;
			}
			return nullptr;
		}
		else {
			return nodes.top ();
		}
	}

	static std::string TreeToInfix_NotAssociative (const binary_tree_node <std::string> & tree) {
		binary_tree_node <std::string> tree_copy (tree);

		Stack <binary_tree_node <std::string> *> nodes;
		Stack <binary_tree_node <std::string> *> parents;

		tree_copy.level_order_traverse (
			[&nodes, &parents]
			(binary_tree_node <std::string> * n, binary_tree_node <std::string> * p) -> void {
				nodes.push (n);
				parents.push (p);
			}
		);

		while (false == nodes.empty ()) {
			auto c = nodes.top ();
			nodes.pop ();
			auto p = parents.top ();
			parents.pop ();

			if (nullptr != c->left && nullptr != c->right) {
				bool needParen = false;

				if ('+' == c->data [0] || '-' == c->data [0]) {
					if (nullptr != p) {
						if ('*' == p->data [0] || '/' == p->data [0]) {
							needParen = true;
						}
					}
				}

				c->data = c->left->data + ' ' + c->data + ' ' + c->right->data;

				if (true == needParen) {
					c->data = '(' + c->data + ')';
				}
			}
		}

		std::string s = tree_copy.data;

		return s;
	}

	static std::string TreeToInfix (const binary_tree_node <std::string> & tree) {
		binary_tree_node <std::string> tree_copy (tree);

		tree_copy.postorder_traverse ([] (
			binary_tree_node <std::string> * node,
			binary_tree_node <std::string> *
		) -> void {
			if (nullptr != node->left && nullptr != node->right) {
				if (true == NotationConverter::NeedParenthesis_BalanceChecker (node->data [0], node->left->data, false)) {
					node->left->data = '(' + node->left->data + ')';
				}

				if (true == NotationConverter::NeedParenthesis_BalanceChecker (node->data [0], node->right->data, true)) {
					node->right->data = '(' + node->right->data + ')';
				}

				node->data = node->left->data + ' ' + node->data + ' ' + node->right->data;
			}
		});

		return tree_copy.data;
	}

	static std::string TreeToInfix_NoBalanceChecker (const binary_tree_node <std::string> & tree) {
		binary_tree_node <std::pair <std::string, std::string>> pair_tree ({
			tree.data, tree.data // orig, result
		});
		Stack <const std::decay_t <decltype (tree)> *> tree_stack;
		tree_stack.push (& tree);
		Stack <decltype (pair_tree) *> pair_tree_stack;
		pair_tree_stack.push (& pair_tree);

		while (false == tree_stack.empty ()) {
			auto t = tree_stack.top ();
			tree_stack.pop ();
			auto p = pair_tree_stack.top ();
			pair_tree_stack.pop ();

			if (nullptr != t->left) {
				p->left = new binary_tree_node (
					std::pair <std::string, std::string> (t->left->data, t->left->data)
				);
				tree_stack.push (t->left);
				pair_tree_stack.push (p->left);
			}

			if (nullptr != t->right) {
				p->right = new binary_tree_node (
					std::pair <std::string, std::string> (t->right->data, t->right->data)
				);
				tree_stack.push (t->right);
				pair_tree_stack.push (p->right);
			}
		}

		pair_tree.postorder_traverse ([] (auto * node, auto *) -> void {
			if (nullptr != node->left && nullptr != node->right) {
				const std::string & lflops = node->left->data.first;
				const std::string & rflops = node->right->data.first;

				std::string nflops = node->data.first;

				char op = nflops [0];

				if (true == NotationConverter::NeedParenthesis (op, lflops, false)) {
					node->left->data.second = '(' + node->left->data.second + ')';
				}
				else {
					nflops += lflops;
				}

				if (true == NotationConverter::NeedParenthesis (op, rflops, true)) {
					node->right->data.second = '(' + node->right->data.second + ')';
				}
				else {
					nflops += rflops;
				}

				node->data.first = nflops;
				node->data.second = node->left->data.second + ' ' + op + ' ' + node->right->data.second;
			}
		});

		return pair_tree.data.second;
	}

	static std::string TreeToPostfix (const binary_tree_node <std::string> & tree) {
		binary_tree_node <std::string> copy (tree);

		copy.postorder_traverse ([] (auto * node, auto *) -> void {
			if (nullptr != node->left && nullptr != node->right) {
				node->data = node->left->data + ' ' + node->right->data + ' ' + node->data;
			}
		});

		return copy.data;
	}
};

# endif // EXPRESSION_TREE_H

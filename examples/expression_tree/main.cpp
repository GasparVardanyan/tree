# include <iostream>
# include <string>

# include <tree/binary_tree_node.hpp>

# include "notation_converter.hpp"
# include "expression_tree.hpp"

int main() {
	std::string res = "(6 * (5 + 3 + 2)) * 10 / 2 - 42";
	std::cout << res << std::endl;

	res = NotationConverter::InfixToPostfix_ShuntingYard (res);
	res.resize (res.find ('='));

	std::cout << res << std::endl;

	auto tree = ExpressionTree::PostfixToTree (res);

	std::cout << ExpressionTree::TreeToPostfix (*tree) << std::endl;
	std::cout << ExpressionTree::TreeToInfix_NotAssociative (*tree) << std::endl;
	std::cout << ExpressionTree::TreeToInfix (*tree) << std::endl;
	std::cout << ExpressionTree::TreeToInfix_NoBalanceChecker (*tree) << std::endl;

	std::cout << *tree << std::endl;

	delete tree;
}

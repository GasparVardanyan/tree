# ifndef NOTATION_CONVERTER_H
# define NOTATION_CONVERTER_H

# include <algorithm>
# include <set>
# include <string>

# include <tree/config.hpp>

# include "balance_checker.hpp"

// NOTE: no negative numbers, no parenthesis, integer division only.
// Expression structural change can happen when it doesn't change the result !!!
// For example a*(b*c) can become a*b*c.
struct NotationConverter : tree::TypeConfig <>::TypeConfigStd {
protected:
	enum class TokenType : int {
		Operator,
		Operand,
		GroupOpener,
		GroupCloser,
		Separator,
	};
	struct Token {
		TokenType type;
		std::string value;
	};

	static inline const TokenType s_symbolType [256] = {
		[' '] = TokenType::Separator,
		['('] = TokenType::GroupOpener,
		[')'] = TokenType::GroupCloser,
		['*'] = TokenType::Operator,
		['+'] = TokenType::Operator,
		['-'] = TokenType::Operator,
		['/'] = TokenType::Operator,
		['0'] = TokenType::Operand,
		['1'] = TokenType::Operand,
		['2'] = TokenType::Operand,
		['3'] = TokenType::Operand,
		['4'] = TokenType::Operand,
		['5'] = TokenType::Operand,
		['6'] = TokenType::Operand,
		['7'] = TokenType::Operand,
		['8'] = TokenType::Operand,
		['9'] = TokenType::Operand,
	};

	struct Operations {
		static int Add (int a, int b) { return a + b; }
		static int Subtract (int a, int b) { return a - b; }
		static int Multiply (int a, int b) { return a * b; }
		static int Divide (int a, int b) { return a / b; }
	};

	typedef int (* Operation) (int, int);

	static inline const Operation OperatorOperations [/* 256 */]  = {
		['+'] = & Operations::Add,
		['-'] = & Operations::Subtract,
		['*'] = & Operations::Multiply,
		['/'] = & Operations::Divide,
	};


public:
	static std::string InfixToPostfix (const std::string & s) {
		Queue <Token> tokens = Tokenize (s);

		Stack <Stack <int>> values;           values.push (Stack <int> ());
		Stack <Stack <char>> operators;       operators.push (Stack <char> ());
		Stack <std::string> results;            results.push ("");

		auto processLast = [&] () -> void {
			Stack <int> & curr_values = values.top ();
			Stack <char> & curr_ops = operators.top ();
			std::string & curr_res = results.top ();

			char op = curr_ops.top ();
			curr_ops.pop ();
			int v2 = curr_values.top ();
			curr_values.pop ();
			int v1 = curr_values.top ();
			curr_values.pop ();
			curr_values.push (OperatorOperations [(unsigned char) op] (v1, v2));

			curr_res += op + std::string {' '};

			// std::cout << "valueStack: {";
			// for (const auto & c : values.container ()) {
			// 	std::cout << '\t';
			// 	for (int i : c.container ()) {
			// 		std::cout << i << ", ";
			// 	}
			// 	std::cout << std::endl;
			// }
			// std::cout << '}' << std::endl;
		};

		auto processValue = [&] (int value, bool newValue = false) -> void {
			Stack <int> & curr_vals = values.top ();
			Stack <char> & curr_ops = operators.top ();
			std::string & curr_res = results.top ();
			curr_vals.push (value);

			if (false == curr_ops.empty ()) {
				char op = curr_ops.top ();

				if ('*' == op || '/' == op) {
					if (true == newValue) {
						curr_res += std::to_string (value) + ' ';
						newValue = false;
					}

					processLast ();
				}

				if (curr_ops.size () > 1) {
					unsigned char last_op = curr_ops.top ();
					curr_ops.pop ();
					int last_v = curr_vals.top ();
					curr_vals.pop ();

					while (false == curr_ops.empty ()) {
						processLast ();
					}

					curr_ops.push (last_op);
					curr_vals.push (last_v);
				}
			}

			if (true == newValue) {
				curr_res += std::to_string (value) + ' ';
			}
		};

		while (false == tokens.empty ()) {
			Token t = tokens.front ();
			tokens.pop ();

			if (TokenType::GroupOpener == t.type) {
				values.push (Stack <int> ());
				operators.push (Stack <char> ());
				results.push ("");
			}
			else if (TokenType::Operator == t.type) {
				operators.top ().push (t.value [0]);
			}
			else if (TokenType::Operand == t.type) {
				processValue (std::stoi (t.value), true);
			}
			else if (TokenType::GroupCloser == t.type) {
				if (false == operators.top ().empty ()) {
					processLast ();
				}

				int v = values.top ().top ();

				values.pop ();
				operators.pop ();
				std::string last = results.top ();
				results.pop ();
				results.top () += last;

				processValue (v);
			}
		}

		Stack <int> & curr_values = values.top ();
		Stack <char> & curr_ops = operators.top ();

		if (false == curr_ops.empty ()) {
			processLast ();
		}

		return results.top () + " = " + std::to_string (curr_values.top ());
	}

	static std::string InfixToPostfix_ShuntingYard (const std::string & s) {
		std::string res = "";

		Stack <char> opStack;

		static const char opPriority [256] = {
			['*'] = 0,
			['/'] = 0,
			['+'] = 1,
			['-'] = 1,
			['('] = 2,
		};

		static const Vector <char> opList {'*', '/', '+', '-'};

		Stack <int> valueStack;

		std::string currOperand;

		auto updateValueStack = [&] (char op) -> void {
			int v2 = valueStack.top ();
			valueStack.pop ();
			int v1 = valueStack.top ();
			valueStack.pop ();

			valueStack.push (OperatorOperations [(unsigned char) op] (v1, v2));
		};

		for (char c : s) {
			if ('0' <= c && c <= '9') {
				currOperand += c;
			}
			else {
				if (false == currOperand.empty ()) {
					res += currOperand + ' ';
					valueStack.push (std::stoi (currOperand));
					currOperand.clear ();
				}
				if ('(' == c) {
					opStack.push (c);
				}
				else if (')' == c) {
					while (false == opStack.empty ()) {
						char t = opStack.top ();
						opStack.pop ();

						if ('(' == t) {
							break;
						}
						else {
							res += t;
							res += ' ';
							updateValueStack (t);
						}
					}
				}
				else if (opList.cend () != std::find (opList.cbegin (), opList.cend (), c)) {
					while (false == opStack.empty ()) {
						char t = opStack.top ();

						int cP = opPriority [(unsigned char) c];
						int tP = opPriority [(unsigned char) t];

						if (cP < tP) {
							break;
						}
						else {
							res += t;
							res += ' ';
							updateValueStack (t);
							opStack.pop ();
						}
					}

					opStack.push (c);
				}
			}
		}

		if (false == currOperand.empty ()) {
			res += currOperand + ' ';
			valueStack.push (std::stoi (currOperand));
		}

		while (false == opStack.empty ()) {
			res += opStack.top ();
			res += ' ';
			updateValueStack (opStack.top ());
			opStack.pop ();
		}

		res += " = " + std::to_string (valueStack.top ());

		return res;
	}

	static std::string PostfixToInfix (const std::string & s) {
		Queue <Token> tokens = Tokenize (s);
		Stack <int> values;
		Stack <std::string> expressions;

		while (false == tokens.empty ()) {
			Token t = tokens.front ();
			tokens.pop ();
			if (TokenType::Operand == t.type) {
				values.push (std::stoi (t.value));
				expressions.push (t.value);
			}
			else {
				if (values.size () >= 2) {
					int o2 = values.top ();
					values.pop ();
					int o1 = values.top ();
					values.pop ();
					values.push (OperatorOperations [(unsigned) t.value [0]] (o1, o2));

					std::string eo2 = expressions.top ();
					expressions.pop ();
					std::string eo1 = expressions.top ();
					expressions.pop ();

					char op = t.value [0];

					if (true == NeedParenthesis_BalanceChecker (op, eo1, false)) {
						eo1 = '(' + eo1 + ')';
					}

					if (true == NeedParenthesis_BalanceChecker (op, eo2, true)) {
						eo2 = '(' + eo2 + ')';
					}

					expressions.push (eo1 + ' ' + t.value + ' ' + eo2);
				}
			}
		}

		return expressions.top () + " = " + std::to_string (values.top ());
	}

protected:
	static Queue <Token> Tokenize (const std::string & s) {
		Queue <Token> tokens;

		bool operand = false;
		std::string curr_operand;

		for (char c : s) {
			TokenType t = s_symbolType [(unsigned char) c];

			if (TokenType::Operand == t) {
				if (false == operand) {
					operand = true;
					curr_operand = c;
				}
				else {
					curr_operand += c;
				}
			}
			else {
				if (true == operand) {
					operand = false;
					tokens.push (Token {
						.type = TokenType::Operand,
						.value = curr_operand
					});
				}

				if (TokenType::Separator != t) {
					tokens.push (Token {
						.type = t,
						.value = std::string {} + c
					});
				}
			}
		}

		if (true == operand) {
			tokens.push (Token {
				.type = TokenType::Operand,
				.value = curr_operand
			});
		}

		return tokens;
	}

	static bool NeedParenthesis_BalanceChecker (char op, std::string str, bool isRightOperand = true) {
		BalanceChecker ch;
		ch.setString (str);
		const auto & parenInfo = ch.pairInfo (BalanceChecker::SymbolType::Parenthesis);
		if (false == parenInfo.empty ()) {
			const auto & firstLevelInfo = parenInfo [0];
			for (const auto & info : firstLevelInfo) {
				std::fill (str.begin () + info.opener_pos, str.begin () + info.closer_pos, ' ');
			}
		}

		std::set <char> flops;

		for (char c : str) {
			if (TokenType::Operator == s_symbolType [(unsigned) c]) {
				flops.insert (c);
			}
		}

		return NeedParenthesis (op, flops, isRightOperand);
	}

	static bool NeedParenthesis (char op, std::set <char> flops, bool isRightOperand = true) {
		bool np = false;

		if ('*' == op) {
			if (true == flops.contains ('-') || flops.contains ('+')) {
				np = true;
			}
		}
		else if ('/' == op) {
			if (false == isRightOperand) {
				if (true == flops.contains ('-') || flops.contains ('+')) {
					np = true;
				}
			}
			else {
				if (false == flops.empty ()) {
					np = true;
				}
			}
		}
		else if ('-' == op) {
			if (true == isRightOperand) {
				if (true == flops.contains ('-') || flops.contains ('+')) {
					np = true;
				}
			}
		}

		return np;
	}

	static bool NeedParenthesis (char op, std::string flops_str, bool isRightOperand = true) {
		std::set <char> flops;

		for (char c : flops_str) {
			if (TokenType::Operator == s_symbolType [(unsigned) c]) {
				flops.insert (c);
			}
		}
		return NeedParenthesis (op, flops, isRightOperand);
	}
};

# endif // NOTATION_CONVERTER_H

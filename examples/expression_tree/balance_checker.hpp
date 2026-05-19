# ifndef BALANCE_CHECKER_H
# define BALANCE_CHECKER_H

# include <sys/types.h>

# include <algorithm>
# include <cstddef>
# include <ostream>
# include <string>
# include <utility>

# include <tree/config.hpp>

class BalanceChecker : tree::TypeConfig <>::TypeConfigStd {
public:
	enum class SymbolType {
		Regular = 0,
		DoubleQuote,
		SingleQuote,
		Parenthesis,
		AngleBracket,
		Bracket,
		BackTick,
		Brace,
		SymbolTypeCount
	};

	struct PairInfo {
		std::size_t opener_pos;
		std::size_t closer_pos;
	};

private:
	enum class SymbolRole : int {
		Regular = 0, Opener, Closer, OpenerCloser
	};

	static const inline SymbolType s_symbolTypes [256] = {
		['"'] = SymbolType::DoubleQuote,
		['\''] = SymbolType::SingleQuote,
		['('] = SymbolType::Parenthesis,
		[')'] = SymbolType::Parenthesis,
		['<'] = SymbolType::AngleBracket,
		['>'] = SymbolType::AngleBracket,
		['['] = SymbolType::Bracket,
		[']'] = SymbolType::Bracket,
		['`'] = SymbolType::BackTick,
		['{'] = SymbolType::Brace,
		['}'] = SymbolType::Brace,
	};

	static const inline SymbolRole s_symbolRoles [256] = {
		['"'] = SymbolRole::OpenerCloser,
		['\''] = SymbolRole::OpenerCloser,
		['('] = SymbolRole::Opener,
		[')'] = SymbolRole::Closer,
		['<'] = SymbolRole::Opener,
		['>'] = SymbolRole::Closer,
		['['] = SymbolRole::Opener,
		[']'] = SymbolRole::Closer,
		['`'] = SymbolRole::OpenerCloser,
		['{'] = SymbolRole::Opener,
		['}'] = SymbolRole::Closer,
	};

	static const inline unsigned char s_openerSymbols [256] = {
		['"'] = '"',
		['\''] = '\'',
		[')'] = '(',
		['>'] = '<',
		[']'] = '[',
		['`'] = '`',
		['}'] = '{',
	};

	struct Data {
		unsigned char symbol;
		std::size_t position;
	};

public:
	struct Status {
		bool success;
		ssize_t error_position;
	};

public:
	BalanceChecker () {
		reset ();
	}

	bool setString (std::string s) {
		m_string = std::move (s);
		return processString ();
	}

	const Vector <Vector <PairInfo>> & pairInfo (SymbolType t) {
		return m_pairInfos [static_cast <std::size_t> (t)];
	}

	friend std::ostream & operator<< (std::ostream & os, BalanceChecker & bc) {
		Status s = bc.m_status;

		if (true == s.success) {
			os << "BalanceChecker [success, " << bc.m_string << "]";
		}
		else {
			std::string es = bc.m_string;
			if (0 <= s.error_position && (std::size_t) s.error_position < es.size ()) {
				os << "BalanceChecker [error, " << es.substr (0, s.error_position) << "HERE" << es.substr (s.error_position) << "]";
			}
		}

		return os;
	}

private:
	bool processString () {
		reset ();

		for (std::size_t i = 0; i < m_string.size (); i++) {
			unsigned char symbol = m_string [i];
			SymbolRole role = s_symbolRoles [symbol];
			SymbolType type = s_symbolTypes [symbol];

			if (true == m_openerCloser) {
				if (SymbolRole::OpenerCloser == role) { // closer
					if (m_stack.top ().symbol == symbol) {
						m_stack.pop ();
						regCloser (type, i);

						m_openerCloser = false;
					}
				}
			}
			else {
				if (SymbolRole::OpenerCloser == role) { // opener
					m_stack.push (Data {
						.symbol = symbol,
						.position = i,
					});
					m_openerCloser = true;

					regOpener (type, i);
				}
				else if (SymbolRole::Opener == role) { // opener
					m_stack.push (Data {
						.symbol = symbol,
						.position = i
					});

					regOpener (type, i);
				}
				else if (SymbolRole::Closer == role) { // closer
					if (false == m_stack.empty () && m_stack.top ().symbol == s_openerSymbols [symbol]) {
						m_stack.pop ();
						regCloser (type, i);
					}
					else {
						m_status.success = false;
						m_status.error_position = i;
						break;
					}
				}

			}
		}

		if (false == m_stack.empty ()) {
			m_status.success = false;
			m_status.error_position = m_stack.top ().position;
		}

		// m_status = s;
		return m_status.success;
	}

	void regOpener (SymbolType type, std::size_t position) {
		std::size_t _type = static_cast <std::size_t> (type);

		if (m_pairInfos [_type].size () < 1 + m_currentLevels [_type]) {
			m_pairInfos [_type].push_back (Vector <PairInfo> {});
		}

		m_pairInfos [_type] [m_currentLevels [_type]].push_back ({
			.opener_pos = position,
			.closer_pos = position,
		});

		m_currentLevels [_type]++;
	}

	void regCloser (SymbolType type, std::size_t position) {
		std::size_t _type = static_cast <std::size_t> (type);
		m_currentLevels [_type]--;
		m_pairInfos [_type] [m_currentLevels [_type]].back ().closer_pos = position;
	}

	void reset () {
		m_openerCloser = false;
		// m_stack.clear ();
		m_stack = {};
		std::fill (
			std::begin (m_pairInfos),
			std::end (m_pairInfos),
			Vector <Vector <PairInfo>> {}
		);
		std::fill (
			std::begin (m_currentLevels),
			std::end (m_currentLevels),
			0
		);
		m_status = {
			.success = true,
			.error_position = 0
		};
	}

private:
	bool m_openerCloser;
	std::string m_string;
	Stack <Data> m_stack;
	Status m_status;
	// symbol type -> level -> info
	Vector <Vector <PairInfo>> m_pairInfos [(std::size_t) SymbolType::SymbolTypeCount];
	std::size_t m_currentLevels [(std::size_t) SymbolType::SymbolTypeCount];
};

# endif // BALANCE_CHECKER_H

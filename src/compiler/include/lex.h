#include <string>
#include <vector>

#include "../lib/BException.hpp"
#include "../lib/Map.hpp"
#include "../lib/Types.hpp"

#include <boost/spirit/home/x3.hpp>

namespace x3 = boost::spirit::x3;
namespace ascii = boost::spirit::x3::ascii;

namespace client::lexing {
	using namespace std;
	using namespace x3;

	/** ========== TOKENS ========== */

	enum class TokenT : u8 {
		NONE,
		KEYWORD,
		SYMBOL,
		IDENT,
		LITERAL
	};

	class Token {
	 public:	
	 	virtual const TokenT getType() { return TokenT::NONE; }
	};

	/** ========== KEYWORDS ========== */

	constexpr auto Keywords = Map<string_view, u8, 6> {{
		array<pair<string_view, u8>, 6> {{
			{"SEQ"sv,   0},
			{"NOT"sv,   1},
			{"WHILE"sv, 2},
			{"IF"sv,    3},
			{"TRUE"sv,  4},
			{"FALSE"sv, 5}
		}}
	}};

	constexpr u8 getKeywordId(string_view keywordSv) { 
		return Keywords.at(keywordSv); 
	}

	struct KeywordParser : x3::symbols<u8> {
		KeywordParser() {
			add("SEQ",   0)
			   ("NOT",   1)
			   ("WHILE", 2)
			   ("IF",    3)
			   ("TRUE",  4)
			   ("FALSE", 5);
		}
	};

	class KeywordToken : public Token {
	 public:
	 	u16 keywordId;

	 	KeywordToken(string_view kwView):
	 		keywordId(getKeywordId(kwView)) {}

	 	const TokenT getType() override { return TokenT::KEYWORD; } 
	};

	rule<Token, u16> keyword = "keyword";
	const auto keyword_def   = new KeywordParser();

	/* ========== SYMBOLS ========== */

	constexpr auto Symbols = Map<string_view, u8, 7>{{
		array<pair<string_view, u8>, 7> {{
			{":="sv, 0},
			{"--"sv, 1},
			{"["sv,  2},
			{"]"sv,  3},
			{"<"sv,  4},
			{"="sv,  5},
			{"<>"sv, 6}
		}}
	}};

	constexpr u8 getSymbolId(string_view  symbolSv) { 
		return Symbols.at(symbolSv) + Keywords.size(); 
	}

	struct SymbolParser : x3::symbols<u8> {
		SymbolParser() {
			add(":=", 0)
				 ("--", 1)
				 ("[",  2)
				 ("]",  3)
				 ("<",  4)
				 ("=",  5)
				 ("<>", 6);
		}
	};

	class SymbolToken : public Token {
	 public:
	 	u8 symbolId;

	 	SymbolToken(string_view symView):
	 		symbolId(getSymbolId(symView)) {}

	 	const TokenT getType() override { return TokenT::SYMBOL; }
	};

	rule<Token, u16> symbol = "symbol";
	const auto symbol_def   = new SymbolParser();

	/** ========== IDENTIFIERS ========== */	

	class IdentToken : public Token {
	 public:
	 	string ident;

	 	IdentToken(string identStr):
	 		ident(identStr) {}

	 	const TokenT getType() override { return TokenT::IDENT; }
	};

	auto initStr   = [&](auto& cxt) { _val(cxt) = ""; }
	auto appendStr = [&](auto& cxt) { _val(cxt).append(_attr(cxt)); } 

	rule<IdentToken, string> ident = "ident";
	const auto ident_def = eps[initStr] >> alpha[appendStr] >> (*alnum[appendStr]);

	class LiteralToken: public Token {
	 public:
	 	string literal;

	 	LiteralToken(string literalStr):
	 		literal(literalStr) {}

	 	const TokenT getType() override { return TokenT::LITERAL; }
	};

	rule<LiteralToken, string> literal = "literal";
	const auto literal_def = eps[initStr] >> (*digit[appendStr]);

	BOOST_SPIRIT_DEFINE(keyword, symbol, ident, literal);

	/* ========== MAIN LEXER ========== */


	class ID;

	vector<Token> toStreamOfTokens(char* first, char* last) { 
		vector<Token> tokenVec;

		/** ----- Callbacks ----- */
		auto onKeyword = [&](auto& cxt) { tokenVec.push_back(KeywordToken(__attr(cxt))); };
		auto onSymbol  = [&](auto& cxt) { tokenVec.push_back(SymbolToken (__attr(cxt))); };
		auto onIdent   = [&](auto& cxt) { tokenVec.push_back(IdentToken  (__attr(cxt))); };
		auto onLiteral = [&](auto& cxt) { tokenVec.push_back(LiteralToken(__attr(cxt))); };

		if (!x3::parse(
			first, 
			last, 
			/** Begin grammar... */
			*(ident[onIdent]     |
				symbol[onSymbol]   | 
				keyword[onKeyword] |
				literal[onLiteral]),
			/** ... end grammar. */
			x3::space
		)) {
			throw BException("Cannot parse input stream into tokens!");
		}

		if (first != last) {
			throw BException("Cannot parse input stream into tokens!");
		}
		
		return tokenVec;
	}
}

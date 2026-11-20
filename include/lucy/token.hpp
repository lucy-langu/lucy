#pragma once
#include <cstddef>
#include <string>
namespace lucy {
enum class TokenType {
 Identifier,Integer,Number,String,Backtick,True,False,Nil,
 If,Unless,Else,While,Do,For,Foreach,Loop,Switch,Case,Default,Function,Def,Func,Lambda,Class,Struct,Return,Break,Continue,End,
 Import,From,As,Const,Let,Var,Global,In,And,Or,Not,New,Self,Super,
 Try,Catch,Finally,Throw,
 Plus,Minus,Star,Slash,Percent,Power,Equal,EqualEqual,BangEqual,StrictEqual,StrictNotEqual,
 Greater,GreaterEqual,Less,LessEqual,Spaceship,BitAnd,BitOr,BitXor,BitNot,ShiftLeft,ShiftRight,
 PlusEqual,MinusEqual,StarEqual,SlashEqual,PercentEqual,PowerEqual,BitAndEqual,BitOrEqual,BitXorEqual,ShiftLeftEqual,ShiftRightEqual,
 Increment,Decrement,RangeInclusive,RangeExclusive,Question,Colon,Dot,
 Coalesce,LeftParen,RightParen,LeftBracket,RightBracket,LeftBrace,RightBrace,Comma,Semicolon,Newline,EndOfFile
};
struct Token {TokenType type;std::string lexeme;std::size_t line=1,column=1;};
std::string token_name(TokenType);
}

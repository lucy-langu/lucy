#pragma once
#include "token.hpp"
#include <string>
#include <vector>
namespace lucy { class Lexer { public: explicit Lexer(std::string s):source_(std::move(s)){} std::vector<Token>scan(); private:std::string source_;size_t current_=0,line_=1,column_=1;std::vector<Token>tokens_;char peek()const;char next()const;char advance();bool match(char);void add(TokenType,std::string);[[noreturn]]void error(const std::string&)const;void identifier();void number();void string();void backtick();}; }

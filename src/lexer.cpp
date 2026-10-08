#include "lucy/lexer.hpp"
#include <cctype>
#include <cstdlib>
#include <string>
#include <unordered_map>
#include <stdexcept>

using namespace lucy;

static const std::unordered_map<std::string,TokenType> K = {
    {"if",TokenType::If},{"unless",TokenType::Unless},{"repeat",TokenType::Do},{"else",TokenType::Else},
    {"while",TokenType::While},{"do",TokenType::Do},{"for",TokenType::For},{"foreach",TokenType::Foreach},
    {"loop",TokenType::Loop},{"switch",TokenType::Switch},{"case",TokenType::Case},{"default",TokenType::Default},
    {"func",TokenType::Func},{"lambda",TokenType::Lambda},{"class",TokenType::Class},{"struct",TokenType::Struct},
    {"return",TokenType::Return},{"self",TokenType::Self},{"super",TokenType::Super},{"try",TokenType::Try},
    {"catch",TokenType::Catch},{"finally",TokenType::Finally},{"throw",TokenType::Throw},{"break",TokenType::Break},
    {"continue",TokenType::Continue},{"import",TokenType::Import},{"from",TokenType::From},{"as",TokenType::As},
    {"const",TokenType::Const},{"let",TokenType::Let},{"var",TokenType::Var},{"global",TokenType::Global},
    {"in",TokenType::In},{"and",TokenType::And},{"or",TokenType::Or},{"not",TokenType::Not},
    {"true",TokenType::True},{"false",TokenType::False},{"nil",TokenType::Nil}
};

char Lexer::peek()const{return current_<source_.size()?source_[current_]:'\0';}
char Lexer::next()const{return current_+1<source_.size()?source_[current_+1]:'\0';}
char Lexer::advance(){char c=peek();if(c){++current_;if(c=='\n'){++line_;column_=1;}else ++column_;}return c;}
bool Lexer::match(char c){if(peek()!=c)return false;advance();return true;}
void Lexer::add(TokenType t,std::string s){tokens_.push_back({t,std::move(s),line_,column_});}
[[noreturn]]void Lexer::error(const std::string&m)const{throw std::runtime_error("SyntaxError: "+m+" at "+std::to_string(line_)+":"+std::to_string(column_));}

static void append_utf8(std::string &out, unsigned long cp)
{
    if (cp <= 0x7f) out.push_back(static_cast<char>(cp));
    else if (cp <= 0x7ff) {
        out.push_back(static_cast<char>(0xc0 | (cp >> 6)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3f)));
    } else if (cp <= 0xffff) {
        out.push_back(static_cast<char>(0xe0 | (cp >> 12)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3f)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3f)));
    } else if (cp <= 0x10ffff) {
        out.push_back(static_cast<char>(0xf0 | (cp >> 18)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3f)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3f)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3f)));
    }
}

void Lexer::identifier(){
    size_t s=current_-1;
    while(std::isalnum((unsigned char)peek())||peek()=='_')advance();
    if(peek()=='?' || (peek()=='!' && next()!='=')) advance();
    auto x=source_.substr(s,current_-s);
    auto it=K.find(x);
    add(it==K.end()?TokenType::Identifier:it->second,x);
}

void Lexer::number(){
    const size_t s = current_ - 1;
    auto validate_separators = [this, s](size_t begin) {
        (void)s;
        const std::string text = source_.substr(begin, current_ - begin);
        if (text.empty() || text.front() == '_' || text.back() == '_' || text.find("__") != std::string::npos)
            error("invalid '_' placement in numeric literal");
    };

    if (source_[s]=='0' && (peek()=='x'||peek()=='X')) {
        advance();
        const size_t digits = current_;
        while (std::isxdigit((unsigned char)peek()) || peek()=='_') advance();
        if (current_==digits) error("hexadecimal literal requires at least one digit");
        validate_separators(digits);
        if (std::isalnum((unsigned char)peek()) || peek()=='_') error("invalid digit in hexadecimal literal");
        add(TokenType::Integer, source_.substr(s,current_-s));
        return;
    }

    if (source_[s]=='0' && (peek()=='b'||peek()=='B')) {
        advance();
        const size_t digits = current_;
        while (std::isalnum((unsigned char)peek()) || peek()=='_') advance();
        if (current_==digits) error("binary literal requires at least one digit");
        validate_separators(digits);
        for (size_t i = digits; i < current_; ++i) {
            const char c = source_[i];
            if (c != '0' && c != '1' && c != '_') error("invalid digit in binary literal");
        }
        add(TokenType::Integer, source_.substr(s,current_-s));
        return;
    }

    while(std::isdigit((unsigned char)peek())||peek()=='_') advance();
    validate_separators(s);

    bool is_float = false;
    if(peek()=='.'&&next()!='.'&&std::isdigit((unsigned char)next())){
        is_float = true;
        advance();
        const size_t fraction = current_;
        while(std::isdigit((unsigned char)peek())||peek()=='_') advance();
        validate_separators(fraction);
    }

    if (peek()=='e' || peek()=='E') {
        is_float = true;
        advance();
        if (peek()=='+' || peek()=='-') advance();
        const size_t exponent = current_;
        while (std::isdigit((unsigned char)peek()) || peek()=='_') advance();
        if (current_==exponent) error("exponent requires at least one digit");
        validate_separators(exponent);
    }

    add(is_float ? TokenType::Number : TokenType::Integer,source_.substr(s,current_-s));
}

void Lexer::string(){
    char quote = source_[current_-1];
    std::string x;
    while(peek()!=quote && peek()!='\0'){
        char c=advance();
        if(c=='\\'){
            char n=peek();
            if (n=='\0') error("unterminated escape sequence");
            advance();
            switch(n){
                case '0': x+='\0'; break;
                case 'b': x+='\b'; break;
                case 'n': x+='\n'; break;
                case 't': x+='\t'; break;
                case 'r': x+='\r'; break;
                case '"': x+='"'; break;
                case '\'': x+='\''; break;
                case '\\': x+='\\'; break;
                case '\n': break;
                case 'x': {
                    std::string h;
                    for(int i=0;i<2;i++) { if(!std::isxdigit((unsigned char)peek())) error("\\x escape requires two hexadecimal digits"); h+=advance(); }
                    x.push_back(static_cast<char>(std::strtoul(h.c_str(),nullptr,16)));
                    break;
                }
                case 'u': {
                    std::string h;
                    for(int i=0;i<4;i++) { if(!std::isxdigit((unsigned char)peek())) error("\\u escape requires four hexadecimal digits"); h+=advance(); }
                    unsigned long cp=std::strtoul(h.c_str(),nullptr,16);
                    if (cp > 0x10ffff || (cp >= 0xd800 && cp <= 0xdfff)) error("invalid Unicode code point");
                    append_utf8(x,cp);
                    break;
                }
                default: error(std::string("unknown escape sequence \\") + n + "'");
            }
        } else x+=c;
    }
    if(!match(quote)) error(std::string("unterminated string; expected closing quote ")+quote);
    add(TokenType::String,x);
}

void Lexer::backtick(){std::string x;while(peek()!='`'&&peek()!='\0')x+=advance();if(!match('`'))error("unterminated backtick command; expected closing '`'");add(TokenType::Backtick,x);}

std::vector<Token>Lexer::scan(){
    bool block=false;
    while(peek()){
        char c=advance();
        if(block){if(c=='='&&source_.compare(current_-1,4,"=end")==0){advance();advance();advance();block=false;}continue;}
        if(c=='='&&source_.compare(current_-1,6,"=begin")==0){for(int i=0;i<5;i++)advance();while(peek()!='\n'&&peek()!='\0')advance();block=true;continue;}
        switch(c){
        case ' ': case '\r': case '\t': break;
        case '\n': add(TokenType::Newline,"\\n"); break;
        case '\\':
            if (peek()=='\n') { advance(); }
            else error("unexpected character '\\\\'");
            break;
        case '#': while(peek()&&peek()!='\n')advance(); break;
        case '"': string(); break;
        case '\'': string(); break;
        case '`': backtick(); break;
        case '(': add(TokenType::LeftParen,"("); break;
        case ')': add(TokenType::RightParen,")"); break;
        case '[': add(TokenType::LeftBracket,"["); break;
        case ']': add(TokenType::RightBracket,"]"); break;
        case '{': add(TokenType::LeftBrace,"{"); break;
        case '}': add(TokenType::RightBrace,"}"); break;
        case ',': add(TokenType::Comma,","); break;
        case ';': add(TokenType::Semicolon,";"); break;
        case '.': if(match('.')){if(match('.'))add(TokenType::RangeExclusive,"...");else add(TokenType::RangeInclusive,"..");}else add(TokenType::Dot,"."); break;
        case '?': if(match('?'))add(TokenType::Coalesce,"??");else if(current_>0&&(std::isalnum((unsigned char)source_[current_-1])||source_[current_-1]=='_'||source_[current_-1]==')'))add(TokenType::Identifier,"?");else add(TokenType::Question,"?"); break;
        case ':': add(TokenType::Colon,":"); break;
        case '+': if(match('+'))add(TokenType::Increment,"++");else if(match('='))add(TokenType::PlusEqual,"+=");else add(TokenType::Plus,"+"); break;
        case '-': if(match('-'))add(TokenType::Decrement,"--");else if(match('='))add(TokenType::MinusEqual,"-=");else add(TokenType::Minus,"-"); break;
        case '*': if(match('*')){if(match('='))add(TokenType::PowerEqual,"**=");else add(TokenType::Power,"**");}else if(match('='))add(TokenType::StarEqual,"*=");else add(TokenType::Star,"*"); break;
        case '/': if(match('='))add(TokenType::SlashEqual,"/=");else add(TokenType::Slash,"/"); break;
        case '%': if(match('='))add(TokenType::PercentEqual,"%=");else add(TokenType::Percent,"%"); break;
        case '!': if(match('=')){if(match('='))add(TokenType::StrictNotEqual,"!==");else add(TokenType::BangEqual,"!=");}else add(TokenType::Not,"!"); break;
        case '=': if(match('=')){if(match('='))add(TokenType::StrictEqual,"===");else add(TokenType::EqualEqual,"==");}else add(TokenType::Equal,"="); break;
        case '>': if(match('='))add(TokenType::GreaterEqual,">=");else if(match('>')){if(match('='))add(TokenType::ShiftRightEqual,">>=");else add(TokenType::ShiftRight,">>");}else add(TokenType::Greater,">"); break;
        case '<': if(match('=')){if(match('>'))add(TokenType::Spaceship,"<=>");else add(TokenType::LessEqual,"<=");}else if(match('<')){if(match('='))add(TokenType::ShiftLeftEqual,"<<=");else add(TokenType::ShiftLeft,"<<");}else add(TokenType::Less,"<"); break;
        case '&': if(match('&'))add(TokenType::And,"&&");else if(match('='))add(TokenType::BitAndEqual,"&=");else add(TokenType::BitAnd,"&"); break;
        case '|': if(match('|'))add(TokenType::Or,"||");else if(match('='))add(TokenType::BitOrEqual,"|=");else add(TokenType::BitOr,"|"); break;
        case '^': if(match('='))add(TokenType::BitXorEqual,"^=");else add(TokenType::BitXor,"^"); break;
        case '~': add(TokenType::BitNot,"~"); break;
        default: if(std::isalpha((unsigned char)c)||c=='_')identifier();else if(std::isdigit((unsigned char)c))number();else error(std::string("unexpected character '")+c+"'");
        }
    }
    if(block) error("unterminated multiline comment; expected =end");
    tokens_.push_back({TokenType::EndOfFile,"",line_,column_});
    return tokens_;
}

std::string lucy::token_name(TokenType t){return std::to_string((int)t);}

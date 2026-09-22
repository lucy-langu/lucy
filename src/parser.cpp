#include "lucy/parser.hpp"
#include <sstream>
#include <stdexcept>
using namespace lucy;

static bool is_name_token(TokenType type)
{
    switch (type)
    {
    case TokenType::Identifier:
    case TokenType::Foreach:
    case TokenType::New:
    case TokenType::Catch:
    case TokenType::Finally:
    case TokenType::Lambda:
    case TokenType::Default:
    case TokenType::Case:
        return true;
    default:
        return false;
    }
}

bool Parser::check(TokenType t) const { return tokens_[current_].type == t; }
bool Parser::match(std::initializer_list<TokenType> ts)
{
    for (auto t : ts)
        if (check(t))
        {
            advance();
            return true;
        }
    return false;
}
Token Parser::advance()
{
    if (!check(TokenType::EndOfFile))
        ++current_;
    return previous();
}
Token Parser::peek() const { return tokens_[current_]; }
Token Parser::previous() const { return tokens_[current_ - 1]; }
Token Parser::consume(TokenType t, const std::string &m)
{
    if (check(t))
        return advance();
    std::ostringstream o;
    o << "SyntaxError: " << m << "; found " << (peek().lexeme.empty() ? "end of input" : "'" + peek().lexeme + "'") << " at line " << peek().line << ", column " << peek().column;
    throw std::runtime_error(o.str());
}
void Parser::skip_newlines()
{
    while (match({TokenType::Newline, TokenType::Semicolon}))
    {
    }
}

// Newlines inside a delimited expression are formatting, not statement boundaries.
// Semicolons intentionally remain significant inside groups.
void Parser::skip_group_newlines()
{
    while (match({TokenType::Newline}))
    {
    }
}

std::vector<StmtPtr> Parser::parse()
{
    std::vector<StmtPtr> out;
    skip_newlines();
    while (!check(TokenType::EndOfFile))
    {
        out.push_back(statement());
        skip_newlines();
    }
    return out;
}
StmtPtr Parser::block_until(std::initializer_list<TokenType> stop)
{
    std::vector<StmtPtr> out;
    skip_newlines();
    while (!check(TokenType::EndOfFile))
    {
        bool done = false;
        for (auto t : stop)
            if (check(t))
                done = true;
        if (done)
            break;
        out.push_back(statement());
        skip_newlines();
    }
    return std::make_shared<Block>(std::move(out));
}

StmtPtr Parser::statement()
{
    // `help topic` is syntax sugar for the Lucy-level REPL help function.
    // The documentation itself lives in stdlib/repl.lucy, not in the runtime.
    if (check(TokenType::Identifier) && peek().lexeme == "help")
    {
        advance();
        if (check(TokenType::Newline) || check(TokenType::Semicolon) || check(TokenType::End) || check(TokenType::EndOfFile))
        {
            return std::make_shared<ExprStmt>(std::make_shared<Variable>("help"));
        }
        if (check(TokenType::String))
        {
            auto topic = std::make_shared<Literal>(advance().lexeme);
            return std::make_shared<ExprStmt>(std::make_shared<Call>(
                std::make_shared<Variable>("help"),
                std::vector<CallArg>{CallArg("", topic)}));
        }
        if (is_name_token(peek().type))
        {
            std::string q = advance().lexeme;
            while (match({TokenType::Dot}))
                q += "." + consume(TokenType::Identifier, "expected help member").lexeme;
            return std::make_shared<ExprStmt>(std::make_shared<Call>(
                std::make_shared<Variable>("help"),
                std::vector<CallArg>{CallArg("", std::make_shared<Literal>(q))}));
        }
        throw std::runtime_error("SyntaxError: expected help topic");
    }
    if (match({TokenType::If}))
        return if_statement();
    if (match({TokenType::While}))
        return while_statement();
    if (match({TokenType::Do}))
        return do_while_statement();
    if (match({TokenType::For}))
        return for_statement();
    if (match({TokenType::Foreach}))
        return foreach_statement();
    if (match({TokenType::Loop}))
        return loop_statement();
    if (match({TokenType::Switch}))
        return switch_statement();
    if (match({TokenType::Function, TokenType::Def}))
        return function_statement();
    if (match({TokenType::Class}))
        return class_statement();
    if (match({TokenType::Try}))
        return try_statement();
    if (match({TokenType::Throw}))
        return std::make_shared<ThrowStmt>(expression());
    if (match({TokenType::Return}))
    {
        if (check(TokenType::Newline) || check(TokenType::Semicolon) || check(TokenType::End) || check(TokenType::EndOfFile))
            return std::make_shared<ReturnStmt>(nullptr);
        return std::make_shared<ReturnStmt>(expression());
    }
    if (match({TokenType::Import}))
        return import_statement();
    if (match({TokenType::From}))
    {
        std::string m = consume(TokenType::Identifier, "expected module name after 'from'").lexeme;
        while (match({TokenType::Dot}))
            m += "." + consume(TokenType::Identifier, "expected module component").lexeme;
        consume(TokenType::Import, "expected 'import' after module name");
        std::vector<std::string> n;
        do
            n.push_back(consume(TokenType::Identifier, "expected imported name").lexeme);
        while (match({TokenType::Comma}));
        return std::make_shared<ImportStmt>(m, n, true);
    }
    if (match({TokenType::Break}))
        return std::make_shared<BreakStmt>();
    if (match({TokenType::Continue}))
        return std::make_shared<ContinueStmt>();
    bool c = false, g = false;
    if (match({TokenType::Const}))
        c = true;
    else if (match({TokenType::Global}))
        g = true;
    if (c || g)
    {
        auto n = consume(TokenType::Identifier, "expected variable name").lexeme;
        if (!match({TokenType::Equal}))
            return std::make_shared<VarDecl>(n, nullptr, c, g);
        return std::make_shared<VarDecl>(n, expression(), c, g);
    }

    if (check(TokenType::Identifier) || check(TokenType::Self) || check(TokenType::LeftBracket))
    {
        size_t save = current_;
        auto left = expression();
        if (assignment(peek().type))
        {
            auto op = advance();
            return std::make_shared<Assign>(left, op, expression());
        }
        current_ = save;
    }
    return std::make_shared<ExprStmt>(expression());
}

bool Parser::command_style_possible() const
{
    TokenType t = peek().type;
    return t == TokenType::String || t == TokenType::Integer || t == TokenType::Number || t == TokenType::True || t == TokenType::False || t == TokenType::Nil || t == TokenType::Identifier || t == TokenType::LeftBracket || t == TokenType::LeftParen || t == TokenType::Backtick || t == TokenType::Not;
}
bool Parser::assignment(TokenType t) const
{
    switch (t)
    {
    case TokenType::Equal:
    case TokenType::PlusEqual:
    case TokenType::MinusEqual:
    case TokenType::StarEqual:
    case TokenType::SlashEqual:
    case TokenType::PercentEqual:
    case TokenType::PowerEqual:
    case TokenType::BitAndEqual:
    case TokenType::BitOrEqual:
    case TokenType::BitXorEqual:
    case TokenType::ShiftLeftEqual:
    case TokenType::ShiftRightEqual:
        return true;
    default:
        return false;
    }
}

StmtPtr Parser::if_statement()
{
    auto c = expression();
    skip_newlines();
    auto t = block_until({TokenType::Else, TokenType::End});
    std::vector<std::pair<ExprPtr, StmtPtr>> ei;
    StmtPtr eb = nullptr;
    while (match({TokenType::Else}))
    {
        if (match({TokenType::If}))
        {
            auto ec = expression();
            skip_newlines();
            ei.push_back({ec, block_until({TokenType::Else, TokenType::End})});
        }
        else
        {
            skip_newlines();
            eb = block_until({TokenType::End});
            break;
        }
    }
    consume(TokenType::End, "expected 'end' to close if");
    return std::make_shared<IfStmt>(c, t, std::move(ei), eb);
}
StmtPtr Parser::while_statement()
{
    auto c = expression();
    skip_newlines();
    auto b = block_until({TokenType::End});
    consume(TokenType::End, "expected 'end' to close while");
    return std::make_shared<WhileStmt>(c, b);
}
StmtPtr Parser::do_while_statement()
{
    skip_newlines();
    auto b = block_until({TokenType::While});
    consume(TokenType::While, "expected 'while' after do block");
    auto c = expression();
    return std::make_shared<DoWhileStmt>(b, c);
}
StmtPtr Parser::switch_statement()
{
    auto v = expression();
    skip_newlines();
    std::vector<std::pair<ExprPtr, StmtPtr>> cases;
    StmtPtr def = nullptr;
    while (!check(TokenType::End) && !check(TokenType::EndOfFile))
    {
        if (match({TokenType::Case}))
        {
            auto cv = expression();
            skip_newlines();
            cases.push_back({cv, block_until({TokenType::Case, TokenType::Default, TokenType::End})});
        }
        else if (match({TokenType::Default}))
        {
            skip_newlines();
            def = block_until({TokenType::End});
            break;
        }
        else
            throw std::runtime_error("SyntaxError: expected 'case', 'default', or 'end' in switch");
        skip_newlines();
    }
    consume(TokenType::End, "expected 'end' to close switch");
    return std::make_shared<SwitchStmt>(v, std::move(cases), def);
}
StmtPtr Parser::for_statement()
{
    auto n = consume(TokenType::Identifier, "expected loop variable after 'for'").lexeme;
    consume(TokenType::In, "expected 'in' after for variable");
    auto it = expression();
    skip_newlines();
    auto b = block_until({TokenType::End});
    consume(TokenType::End, "expected 'end' to close for");
    return std::make_shared<ForStmt>(n, it, b);
}
StmtPtr Parser::foreach_statement()
{
    auto n = consume(TokenType::Identifier, "expected loop variable after 'foreach'").lexeme;
    consume(TokenType::In, "expected 'in' after foreach variable");
    auto it = expression();
    skip_newlines();
    auto b = block_until({TokenType::End});
    consume(TokenType::End, "expected 'end' to close foreach");
    return std::make_shared<ForStmt>(n, it, b);
}
StmtPtr Parser::loop_statement()
{
    skip_newlines();
    auto b = block_until({TokenType::End});
    consume(TokenType::End, "expected 'end' to close loop");
    return std::make_shared<LoopStmt>(b);
}

StmtPtr Parser::function_statement()
{
    if (!is_name_token(peek().type))
    {
        throw std::runtime_error("SyntaxError: expected function name; found '" + peek().lexeme +
                                 "' at line " + std::to_string(peek().line) +
                                 ", column " + std::to_string(peek().column));
    }
    auto n = advance().lexeme;
    std::vector<Parameter> params;
    bool default_seen = false;
    auto parse_param = [&]()
    {
        bool variadic = match({TokenType::Star});
        std::string name;
        if (!is_name_token(peek().type))
            throw std::runtime_error("SyntaxError: expected parameter name; found '" + peek().lexeme + "'");
        name = advance().lexeme;
        ExprPtr def = nullptr;
        if (match({TokenType::Equal}))
        {
            if (variadic)
                throw std::runtime_error("SyntaxError: variadic parameter cannot have a default value");
            default_seen = true;
            def = expression();
        }
        else if (default_seen && !variadic)
            throw std::runtime_error("SyntaxError: required parameter '" + name + "' cannot follow a parameter with a default value");
        return Parameter(name, def, variadic);
    };
    if (match({TokenType::LeftParen}))
    {
        skip_group_newlines();
        if (!check(TokenType::RightParen))
        {
            do
            {
                skip_group_newlines();
                params.push_back(parse_param());
                if (params.back().variadic && check(TokenType::Comma))
                    throw std::runtime_error("SyntaxError: variadic parameter must be last");
                skip_group_newlines();
            } while (match({TokenType::Comma}));
        }
        skip_group_newlines();
        consume(TokenType::RightParen, "expected ')' after parameters");
    }
    else if (!check(TokenType::Newline) && !check(TokenType::Semicolon))
    {
        do
        {
            params.push_back(parse_param());
            if (params.back().variadic && check(TokenType::Comma))
                throw std::runtime_error("SyntaxError: variadic parameter must be last");
        } while (match({TokenType::Comma}));
    }
    skip_newlines();
    auto b = block_until({TokenType::End});
    consume(TokenType::End, "expected 'end' to close function");
    return std::make_shared<FunctionStmt>(n, std::move(params), b);
}

StmtPtr Parser::class_statement()
{
    auto n = consume(TokenType::Identifier, "expected class name").lexeme;
    std::string base;
    if (match({TokenType::Less}))
        base = consume(TokenType::Identifier, "expected base class name after '<'").lexeme;
    skip_newlines();
    std::vector<std::shared_ptr<FunctionStmt>> m;
    while (!check(TokenType::End) && !check(TokenType::EndOfFile))
    {
        if (!match({TokenType::Function, TokenType::Def}))
            throw std::runtime_error("SyntaxError: class body may only contain methods at line " + std::to_string(peek().line));
        m.push_back(std::dynamic_pointer_cast<FunctionStmt>(function_statement()));
        skip_newlines();
    }
    consume(TokenType::End, "expected 'end' to close class");
    return std::make_shared<ClassStmt>(n, base, std::move(m));
}
StmtPtr Parser::try_statement()
{
    skip_newlines();
    auto b = block_until({TokenType::Catch, TokenType::Finally, TokenType::End});
    std::string type, name;
    StmtPtr cb = nullptr, fb = nullptr;
    if (match({TokenType::Catch}))
    {
        if (check(TokenType::Identifier))
        {
            type = advance().lexeme;
            if (check(TokenType::As))
            {
                advance();
                name = consume(TokenType::Identifier, "expected exception variable name after 'as'").lexeme;
            }
        }
        skip_newlines();
        cb = block_until({TokenType::Finally, TokenType::End});
    }
    if (match({TokenType::Finally}))
    {
        skip_newlines();
        fb = block_until({TokenType::End});
    }
    consume(TokenType::End, "expected 'end' to close try");
    return std::make_shared<TryStmt>(b, type, name, cb, fb);
}
StmtPtr Parser::import_statement()
{
    std::string m = consume(TokenType::Identifier, "expected module name after 'import'").lexeme;
    while (match({TokenType::Dot}))
        m += '.' + consume(TokenType::Identifier, "expected module component").lexeme;
    std::string a;
    if (match({TokenType::As}))
        a = consume(TokenType::Identifier, "expected alias after 'as'").lexeme;
    return std::make_shared<ImportStmt>(m, std::vector<std::string>{}, false, a);
}

ExprPtr Parser::expression() { return ternary(); }
ExprPtr Parser::ternary()
{
    auto e = logical_or();
    if (match({TokenType::Question}))
    {
        auto a = expression();
        consume(TokenType::Colon, "expected ':' in ternary expression");
        return std::make_shared<Ternary>(e, a, expression());
    }
    return e;
}
ExprPtr Parser::logical_or()
{
    auto e = logical_and();
    while (match({TokenType::Or}))
    {
        auto o = previous();
        e = std::make_shared<Binary>(e, o, logical_and());
    }
    return e;
}
ExprPtr Parser::logical_and()
{
    auto e = bit_or();
    while (match({TokenType::And}))
    {
        auto o = previous();
        e = std::make_shared<Binary>(e, o, bit_or());
    }
    return e;
}
ExprPtr Parser::bit_or()
{
    auto e = bit_xor();
    while (match({TokenType::BitOr}))
    {
        auto o = previous();
        e = std::make_shared<Binary>(e, o, bit_xor());
    }
    return e;
}
ExprPtr Parser::bit_xor()
{
    auto e = bit_and();
    while (match({TokenType::BitXor}))
    {
        auto o = previous();
        e = std::make_shared<Binary>(e, o, bit_and());
    }
    return e;
}
ExprPtr Parser::bit_and()
{
    auto e = equality();
    while (match({TokenType::BitAnd}))
    {
        auto o = previous();
        e = std::make_shared<Binary>(e, o, equality());
    }
    return e;
}
ExprPtr Parser::equality()
{
    auto e = comparison();
    while (match({TokenType::EqualEqual, TokenType::BangEqual, TokenType::StrictEqual, TokenType::StrictNotEqual}))
    {
        auto o = previous();
        e = std::make_shared<Binary>(e, o, comparison());
    }
    return e;
}
ExprPtr Parser::comparison()
{
    auto e = shift();
    while (match({TokenType::Greater, TokenType::GreaterEqual, TokenType::Less, TokenType::LessEqual, TokenType::Spaceship, TokenType::In}))
    {
        auto o = previous();
        e = std::make_shared<Binary>(e, o, shift());
    }
    return e;
}
ExprPtr Parser::shift()
{
    auto e = range();
    while (match({TokenType::ShiftLeft, TokenType::ShiftRight}))
    {
        auto o = previous();
        e = std::make_shared<Binary>(e, o, range());
    }
    return e;
}
ExprPtr Parser::range()
{
    auto e = term();
    if (match({TokenType::RangeInclusive, TokenType::RangeExclusive}))
    {
        auto o = previous();
        return std::make_shared<RangeExpr>(e, term(), o.type == TokenType::RangeInclusive);
    }
    return e;
}
ExprPtr Parser::term()
{
    auto e = factor();
    while (match({TokenType::Plus, TokenType::Minus}))
    {
        auto o = previous();
        e = std::make_shared<Binary>(e, o, factor());
    }
    return e;
}
ExprPtr Parser::factor()
{
    auto e = power();
    while (match({TokenType::Star, TokenType::Slash, TokenType::Percent}))
    {
        auto o = previous();
        e = std::make_shared<Binary>(e, o, power());
    }
    return e;
}
ExprPtr Parser::power()
{
    auto e = unary();
    if (match({TokenType::Power}))
    {
        auto o = previous();
        e = std::make_shared<Binary>(e, o, power());
    }
    return e;
}
ExprPtr Parser::unary()
{
    if (match({TokenType::Not, TokenType::Minus, TokenType::Plus, TokenType::BitNot, TokenType::Increment, TokenType::Decrement}))
    {
        auto o = previous();
        return std::make_shared<Unary>(o, unary());
    }
    return postfix();
}

ExprPtr Parser::postfix()
{
    auto e = primary();
    while (true)
    {
        if (match({TokenType::LeftParen}))
        {
            std::vector<CallArg> a;
            bool named_seen = false;
            skip_group_newlines();
            if (!check(TokenType::RightParen))
                do
                {
                    skip_group_newlines();
                    if (check(TokenType::Identifier) && current_ + 1 < tokens_.size() && tokens_[current_ + 1].type == TokenType::Colon)
                    {
                        std::string name = advance().lexeme;
                        advance();
                        a.emplace_back(name, expression());
                        named_seen = true;
                    }
                    else
                    {
                        if (named_seen)
                            throw std::runtime_error("SyntaxError: positional argument cannot follow named argument at line " + std::to_string(peek().line));
                        a.emplace_back("", expression());
                    }
                    skip_group_newlines();
                } while (match({TokenType::Comma}));

            skip_group_newlines();
            consume(TokenType::RightParen, "expected ')' after arguments");
            e = std::make_shared<Call>(e, std::move(a));
        }
        else if (match({TokenType::LeftBracket}))
        {
            skip_group_newlines();
            auto i = expression();
            skip_group_newlines();
            consume(TokenType::RightBracket, "expected ']' after index");
            e = std::make_shared<Index>(e, i);
        }
        else if (match({TokenType::Dot}))
        {
            if (!is_name_token(peek().type))
                throw std::runtime_error("SyntaxError: expected member name after '.'; found '" + peek().lexeme + "'");
            auto n = advance().lexeme;
            e = std::make_shared<Member>(e, n);
        }
        else if (match({TokenType::Increment, TokenType::Decrement}))
        {
            e = std::make_shared<Unary>(previous(), e, true);
        }
        else if ((std::dynamic_pointer_cast<Variable>(e) || std::dynamic_pointer_cast<Member>(e)) && command_style_possible())
        {
            auto callee = e;
            std::vector<CallArg> a;
            a.emplace_back("", expression());
            while (match({TokenType::Comma}))
                a.emplace_back("", expression());
            e = std::make_shared<Call>(callee, std::move(a));
        }
        else
            break;
    }
    return e;
}

ExprPtr Parser::lambda_expression()
{
    std::vector<Parameter> params;
    if (match({TokenType::LeftParen}))
    {
        skip_group_newlines();
        if (!check(TokenType::RightParen))
        {
            do
            {
                skip_group_newlines();
                std::string n = consume(TokenType::Identifier, "expected lambda parameter name").lexeme;
                ExprPtr d = nullptr;
                if (match({TokenType::Equal}))
                    d = expression();
                params.emplace_back(n, d);
                skip_group_newlines();
            } while (match({TokenType::Comma}));
        }
        skip_group_newlines();
        consume(TokenType::RightParen, "expected ')' after lambda parameters");
    }
    else
    {
        std::string n = consume(TokenType::Identifier, "expected lambda parameter name").lexeme;
        params.emplace_back(n);
    }
    consume(TokenType::Equal, "expected '=' after lambda parameters");
    consume(TokenType::Greater, "expected '>' after lambda '='");
    return std::make_shared<LambdaExpr>(std::move(params), expression());
}
ExprPtr Parser::primary()
{
    if (match({TokenType::Lambda}))
        return lambda_expression();
    if (match({TokenType::False}))
        return std::make_shared<Literal>(false);
    if (match({TokenType::True}))
        return std::make_shared<Literal>(true);
    if (match({TokenType::Nil}))
        return std::make_shared<Literal>(Value{});
    if (match({TokenType::Integer}))
        return std::make_shared<Literal>(std::stoll(previous().lexeme));
    if (match({TokenType::Number}))
        return std::make_shared<Literal>(std::stod(previous().lexeme));
    if (match({TokenType::String}))
        return std::make_shared<Literal>(previous().lexeme);
    if (match({TokenType::Backtick}))
        return std::make_shared<ShellExpr>(previous().lexeme);
    if (match({TokenType::Self}))
        return std::make_shared<Variable>("self");
    if (match({TokenType::New}))
        return std::make_shared<Variable>("new");
    if (match({TokenType::Identifier, TokenType::Default, TokenType::Case}))
        return std::make_shared<Variable>(previous().lexeme);
    if (match({TokenType::LeftBracket}))
    {
        std::vector<ExprPtr> a;
        skip_group_newlines();
        if (!check(TokenType::RightBracket))
        {
            do
            {
                skip_group_newlines();
                a.push_back(expression());
                skip_group_newlines();
            } while (match({TokenType::Comma}));
        }
        skip_group_newlines();
        consume(TokenType::RightBracket, "expected ']' after array");
        return std::make_shared<ArrayExpr>(std::move(a));
    }
    if (match({TokenType::LeftBrace}))
    {
        std::vector<std::pair<std::string, ExprPtr>> a;
        skip_group_newlines();
        if (!check(TokenType::RightBrace))
        {
            do
            {
                skip_group_newlines();
                std::string k;
                if (!is_name_token(peek().type))
                    throw std::runtime_error("SyntaxError: expected map key");
                k = advance().lexeme;
                if (match({TokenType::Colon}))
                    a.push_back({k, expression()});
                else
                    throw std::runtime_error("SyntaxError: expected ':' after map key");
                skip_group_newlines();
            } while (match({TokenType::Comma}));
        }
        skip_group_newlines();
        consume(TokenType::RightBrace, "expected '}' after map");
        return std::make_shared<MapExpr>(std::move(a));
    }
    if (match({TokenType::LeftParen}))
    {
        skip_group_newlines();
        auto e = expression();
        skip_group_newlines();
        consume(TokenType::RightParen, "expected ')' after expression");
        return e;
    }
    throw std::runtime_error("SyntaxError: expected expression at line " + std::to_string(peek().line) + ", column " + std::to_string(peek().column));
}

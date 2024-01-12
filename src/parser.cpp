#include "parser.h"

Parser::Parser(std::vector<Token> tokens)
    : _tokens(std::move(tokens))
    , _allocator(1024 * 1024 * 4) 
    {} // 4mb

std::string to_string(VarType t)
{
    switch (t)
    {
    case VarType::NONE:
        return "";
    case VarType::INT:
        return "int";
    default:
        return "auto";
    }
}

VarType to_variable_type(TokenType t)
{
    switch (t)
    {
    case TokenType::TYPE_INT:
    case TokenType::INTEGER_LITERAL:
        return VarType::INT;
    default:
        return VarType::NONE;
    }
}

std::optional<Token> Parser::peek(const int offset) const
{
    if (_index + offset >= _tokens.size())
        return {};
    return _tokens.at(_index + offset);
}

bool Parser::peek_type(TokenType type, int offset) const
{
    return peek(offset).has_value() && peek(offset).value().type == type;
}

Token Parser::consume()
{
    return _tokens[_index++];
}

Token Parser::try_consume_err(TokenType type)
{
    if (peek().has_value() && peek().value().type == type)
    {
        return consume();
    }
    else
    {
        exit_with(to_string(type));
        return {}; // unreachable
    }
}

std::optional<Token> Parser::try_consume(TokenType type)
{
    if (peek().has_value() && peek().value().type == type)
    {
        return consume();
    }
    else
    {
        return {};
    }
}

void Parser::exit_with(const std::string &err_msg, std::string template_msg)
{
    std::cerr << "[Parse Error] " << template_msg << " " << err_msg << " on line ";

    if (peek().has_value())
        std::cerr << peek().value().line;
    else
        std::cerr << peek(-1).value().line;

    std::cerr << std::endl;

    exit(EXIT_FAILURE);
}

std::optional<Node::Prog> Parser::parse_prog()
{
    Node::Prog prog;

    while (peek().has_value())
    {
        if (std::optional<Node::Stmt *> stmt = parse_stmt())
        {
            prog.stmts.push_back(stmt.value());
        }
        else
        {
            exit_with("statement");
        }
    }

    return prog;
};

std::optional<Node::Stmt *> Parser::parse_stmt()
{
    if (!peek().has_value())
        return {};

    // RETURN ?
    if (peek_type(TokenType::RETURN))
    {
        consume();
        Node::StmtReturn *ret = _allocator.emplace<Node::StmtReturn>();

        if (auto ne = parse_expr())
            ret->expr = ne.value();
        else
            exit_with("return value");

        Node::Stmt *stmt = _allocator.emplace<Node::Stmt>(ret);

        return stmt;
    }

    // VAR IDENT = ?
    if (peek_type(TokenType::VAR) && peek_type(TokenType::IDENTIFIER, 1))
    {
        if (peek_type(TokenType::EQUAL, 2))
        {
            consume();
            Node::StmtImplicitVar *var = _allocator.emplace<Node::StmtImplicitVar>();
            var->identifier = consume();
            consume();
            if (auto e = parse_expr())
            {
                var->expr = e.value();
            }
            else
            {
                exit_with("expression");
            }

            Node::Stmt *stmt = _allocator.emplace<Node::Stmt>(var);
            return stmt;
        }
        else if (peek_type(TokenType::COLON, 2) && peek_type(TokenType::EQUAL, 4))
        {
            consume();
            Node::StmtImplicitVar *var = _allocator.emplace<Node::StmtImplicitVar>();
            var->identifier = consume();
            consume();
            Token type = consume();
            consume();
            if (auto e = parse_expr())
            {
                var->expr = e.value();
            }
            else
            {
                exit_with("expression");
            }

            if (var->expr->type != to_variable_type(type.type))
                exit_with(to_string(type.type), "variable value type must be");

            Node::Stmt *stmt = _allocator.emplace<Node::Stmt>(var);
            return stmt;
        }
        else if (peek_type(TokenType::COLON, 2))
        {
            consume();
            Node::StmtExplicitVar *var = _allocator.emplace<Node::StmtExplicitVar>();
            var->ident = consume();
            consume();
            if (auto t = parse_type())
            {
                var->type = t.value();
            }
            else
            {
                exit_with("type");
            }

            Node::Stmt *stmt = _allocator.emplace<Node::Stmt>(var);
            return stmt;
        }
        else exit_with("type declaration");
    }

    // IDENT = ?
    if (peek_type(TokenType::IDENTIFIER) && peek_type(TokenType::EQUAL, 1))
    {
        auto var_assign = _allocator.alloc<Node::StmtVarAssign>();
        var_assign->ident = consume();

        consume(); // = token

        if (const auto expr = parse_expr())
        {
            var_assign->expr = expr.value();
        }
        else
            exit_with("expression");

        return _allocator.emplace<Node::Stmt>(var_assign);
    }

    // IDENT(?)
    if (peek_type(TokenType::IDENTIFIER) && peek_type(TokenType::LEFT_PARENTHESIS, 1))
    {
        auto fcall = _allocator.alloc<Node::StmtFuncCall>();
        fcall->ident = consume();

        consume(); // ( token

        fcall->args = parse_args();

        try_consume_err(TokenType::RIGHT_PARENTHESIS);

        return _allocator.emplace<Node::Stmt>(fcall);
    }

    // { ? }
    if (peek_type(TokenType::LEFT_CURLY_BACKET))
    {
        if (auto scope = parse_scope())
        {
            auto stmt = _allocator.emplace<Node::Stmt>(scope.value());
            return stmt;
        }
        else
            exit_with("scope");
    }

    // IF ( ? ) ?
    if (auto tif = try_consume(TokenType::IF))
    {
        try_consume_err(TokenType::LEFT_PARENTHESIS);

        auto stmt_if = _allocator.emplace<Node::StmtIf>();

        if (auto expr = parse_expr())
        {
            stmt_if->expr = expr.value();
        }
        else
            exit_with("expression");

        try_consume_err(TokenType::RIGHT_PARENTHESIS);

        if (auto scope = parse_scope())
        {
            stmt_if->scope = scope.value();
        }
        else
            exit_with("scope");

        stmt_if->pred = parse_if_pred();

        auto stmt = _allocator.emplace<Node::Stmt>(stmt_if);
        return stmt;
    }

    return {};
}

std::optional<Node::Scope *> Parser::parse_scope()
{
    if (!try_consume(TokenType::LEFT_CURLY_BACKET).has_value())
        return {};

    auto scope = _allocator.emplace<Node::Scope>();
    
    while (auto stmt = parse_stmt())
    {
        scope->stmts.push_back(stmt.value());
    }

    try_consume_err(TokenType::RIGHT_CURLY_BRACKET);

    return scope;
}

std::optional<Node::ArgList*> Parser::parse_args()
{
    if (const auto e = parse_expr())
    {
        auto a = _allocator.emplace<Node::ArgList>(e.value());
        a->next_arg = parse_args();
        return a;
    }
    else return {};
}

std::optional<Node::IfPred *> Parser::parse_if_pred()
{
    if (auto t = try_consume(TokenType::ELIF))
    {
        try_consume_err(TokenType::LEFT_PARENTHESIS);
        auto elif_pred = _allocator.alloc<Node::IfPredElif>();
        if (const auto expr = parse_expr())
            elif_pred->expr = expr.value();
        else
            exit_with("expression");

        try_consume_err(TokenType::RIGHT_PARENTHESIS);

        if (const auto scope = parse_scope())
            elif_pred->scope = scope.value();
        else
            exit_with("scope");

        elif_pred->pred = parse_if_pred();

        return _allocator.emplace<Node::IfPred>(elif_pred);
    }

    if (try_consume(TokenType::ELSE))
    {
        auto else_pred = _allocator.alloc<Node::IfPredElse>();
        if (const auto scope = parse_scope())
            else_pred->scope = scope.value();
        else
            exit_with("scope");

        return _allocator.emplace<Node::IfPred>(else_pred);
    }

    return {};
}

std::optional<Node::Expr *> Parser::parse_expr(int min_prec)
{
    std::optional<Node::Term *> lterm = parse_term();
    if (!lterm.has_value())
        return {};

    auto expr = _allocator.emplace<Node::Expr>(lterm.value(), lterm.value()->type);

    while (true)
    {
        std::optional<Token> curr_tok = peek();
        std::optional<int> prec;

        if (curr_tok.has_value())
        {
            prec = bin_prec(curr_tok.value().type);
            if (!prec.has_value() || prec.value() < min_prec)
            {
                break;
            }
        }
        else
            break;

        Token op = consume();

        int next_min_prec = prec.value() + 1;
        auto expr_rside = parse_expr(next_min_prec);
        if (!expr_rside.has_value())
            exit_with("expression");

        if (expr->type != VarType::NONE &&
            expr_rside.value()->type != VarType::NONE &&
            expr->type != expr_rside.value()->type)
        {
            exit_with(to_string(expr->type) + to_string(op.type) + to_string(expr_rside.value()->type),
                        "wrong operation :");
        }

        auto bin_expr = _allocator.emplace<Node::BinExpr>();
        auto expr_lside = _allocator.emplace<Node::Expr>(expr->var);

        if (op.type == TokenType::PLUS)
        {
            auto add = _allocator.emplace<Node::BinExprAdd>(expr_lside, expr_rside.value());
            bin_expr->var = add;
        }
        else if (op.type == TokenType::MINUS)
        {
            /// TODO: make sure you cannot sub strings
            auto sub = _allocator.emplace<Node::BinExprSub>(expr_lside, expr_rside.value());
            bin_expr->var = sub;
        }
        else if (op.type == TokenType::STAR)
        {
            auto multi = _allocator.emplace<Node::BinExprMulti>(expr_lside, expr_rside.value());
            bin_expr->var = multi;
        }
        else if (op.type == TokenType::SLASH)
        {
            /// TODO: make sure you cannot divide strings
            auto div = _allocator.emplace<Node::BinExprDiv>(expr_lside, expr_rside.value());
            bin_expr->var = div;
        }
        else
            assert(false); // unreachable

        expr->var = bin_expr;
    }

    return expr;
}

std::optional<Node::Term *> Parser::parse_term()
{

    if (auto int_lit = try_consume(TokenType::INTEGER_LITERAL))
    {
        auto term_int_lit = _allocator.emplace<Node::TermIntegerLiteral>(int_lit.value());
        auto term = _allocator.emplace<Node::Term>(term_int_lit, VarType::INT);
        return term;
    }

    if (auto ident = try_consume(TokenType::IDENTIFIER))
    {
        auto expr_ident = _allocator.emplace<Node::TermIdentifier>(ident.value());
        auto term = _allocator.emplace<Node::Term>(expr_ident, to_variable_type(ident.value().type));
        return term;
    }

    if (auto open_paren = try_consume(TokenType::LEFT_PARENTHESIS))
    {
        auto expr = parse_expr();
        if (!expr.has_value())
            exit_with("expression");

        try_consume_err(TokenType::RIGHT_PARENTHESIS);

        auto term_paren = _allocator.emplace<Node::TermParen>(expr.value());
        auto term = _allocator.emplace<Node::Term>(term_paren, expr.value()->type);
        return term;
    }

    return {};
}

std::optional<VarType> Parser::parse_type()
{
    if (auto t = try_consume(TokenType::TYPE_INT))
        return VarType::INT;
    
    return {};
}

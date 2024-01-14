#include "parser.h"

#include "buildin.h"

std::string to_string(VarType t)
{
    switch (t)
    {
    case VarType::VOID:
        return "void";
    case VarType::INT:
        return "int";
    case VarType::CHAR:
        return "char";
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
    case TokenType::TYPE_CHAR:
    case TokenType::CHAR_LITERAL:
        return VarType::CHAR;
    default:
        return VarType::VOID;
    }
}

Parser::Parser(std::vector<Token> tokens)
    : tokens(std::move(tokens)), allocator(1024 * 1024 * 4)
{
} // 4mb

const std::unordered_map<std::string, VarType> Parser::buildin_func_type = {
    {"print", VarType::VOID},
    {"println", VarType::VOID},

    {"itoc", VarType::CHAR},
    {"ctoi", VarType::INT},
};

bool Parser::is_buildin_func(std::string func)
{
    return buildin_func_type.contains(func);
}

std::optional<VarType> Parser::var_type(const std::string &ident)
{
    if (identifiers.contains(ident))
        return identifiers[ident];
    return {};
}

std::optional<Token> Parser::peek(const int offset) const
{
    if (index + offset >= tokens.size())
        return {};
    return tokens.at(index + offset);
}

bool Parser::peek_type(TokenType type, int offset) const
{
    return peek(offset).has_value() && peek(offset).value().type == type;
}

Token Parser::consume()
{
    return tokens[index++];
}

Token Parser::try_consume_err(TokenType type)
{
    if (peek().has_value() && peek().value().type == type)
    {
        return consume();
    }
    else
    {
        exit_with("`" + to_string(type) + "`");
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
    std::cerr << "[Error] " << template_msg << " " << err_msg << " on line ";

    if (peek().has_value())
        std::cerr << peek().value().line;
    else
        std::cerr << peek(-1).value().line;

    std::cerr << std::endl;

    exit(EXIT_FAILURE);
}

/* ----- PARSING FUNCTIONS ----- */

std::optional<Node::Prog> Parser::parse_prog()
{
    Node::Prog prog;

    while (peek().has_value())
    {
        if (std::optional<Node::ProgStmt *> stmt = parse_prog_stmt())
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

std::optional<Node::ProgStmt *> Parser::parse_prog_stmt()
{
    // VAR IDENT ?
    if (peek_type(TokenType::VAR) && peek_type(TokenType::IDENTIFIER, 1))
    {
        // VAR IDENT = ?
        if (peek_type(TokenType::EQUAL, 2))
        {
            consume(); // var

            Node::StmtImplicitVar *var = allocator.emplace<Node::StmtImplicitVar>();
            var->identifier = consume();

            if (identifiers.contains(var->identifier.val.value()))
                exit_with("'" + var->identifier.val.value() + "' already used", "identifier");

            consume(); // =

            if (auto e = parse_expr())
            {
                var->expr = e.value();
            }
            else
            {
                exit_with("expression");
            }

            identifiers[var->identifier.val.value()] = var->expr->type;

            Node::ProgStmt *stmt = allocator.emplace<Node::ProgStmt>(var);
            return stmt;
        }
        // VAR IDENT : TYPE = ?
        else if (peek_type(TokenType::COLON, 2) && peek_type(TokenType::EQUAL, 4))
        {
            consume(); // var

            Node::StmtImplicitVar *var = allocator.emplace<Node::StmtImplicitVar>();
            var->identifier = consume();

            if (identifiers.contains(var->identifier.val.value()))
                exit_with("'" + var->identifier.val.value() + "' already used", "identifier");

            consume(); // :
            Token type = consume();
            consume(); // =

            if (auto e = parse_expr())
            {
                var->expr = e.value();
            }
            else
            {
                exit_with("expression");
            }

            if (var->expr->type != to_variable_type(type.type))
                exit_with(to_string(type.type), "variable type must be");

            identifiers[var->identifier.val.value()] = var->expr->type;

            Node::ProgStmt *stmt = allocator.emplace<Node::ProgStmt>(var);
            return stmt;
        }
        // VAR IDENT : TYPE
        else if (peek_type(TokenType::COLON, 2))
        {
            consume(); // var
            Node::StmtExplicitVar *var = allocator.emplace<Node::StmtExplicitVar>();
            var->ident = consume();

            if (identifiers.contains(var->ident.val.value()))
                exit_with("'" + var->ident.val.value() + "' already used", "identifier");

            consume(); // :

            if (auto t = parse_type())
            {
                var->type = t.value();
            }
            else
            {
                exit_with("type");
            }

            identifiers[var->ident.val.value()] = var->type;

            Node::ProgStmt *stmt = allocator.emplace<Node::ProgStmt>(var);
            return stmt;
        }
        else
            exit_with("type declaration");
    }

    // FUNC IDENT() ?
    if (peek_type(TokenType::FUNC))
    {
        consume();

        auto func = allocator.alloc<Node::FuncDeclaration>();
        func->ident = try_consume_err(TokenType::IDENTIFIER);

        try_consume_err(TokenType::LEFT_PARENTHESIS);
        try_consume_err(TokenType::RIGHT_PARENTHESIS);

        if (peek_type(TokenType::COLON))
        {
            consume(); // :

            if (const auto t = parse_type())
                func->type = t.value();
            else
                exit_with("type specifier");

            if (const auto s = parse_scope())
            {
                func->scope = s.value();
            }
            else
            {
                exit_with("scope");
                return {}; // unreachable
            }

            if (func->type != func->scope->type)
                exit_with(func->ident.val.value() + " is of type " + to_string(func->type), "function");

            return allocator.emplace<Node::ProgStmt>(func);
        }

        if (const auto s = parse_scope())
        {
            func->scope = s.value();
        }
        else
        {
            exit_with("scope");
            return {}; // unreachable
        }

        func->type = func->scope->type;

        return allocator.emplace<Node::ProgStmt>(func);
    }

    return {};
}

std::optional<Node::Scope *> Parser::parse_scope()
{
    if (!try_consume(TokenType::LEFT_CURLY_BACKET).has_value())
        return {};

    auto scope = allocator.emplace<Node::Scope>();

    while (auto stmt = parse_scope_stmt())
    {
        scope->stmts.push_back(stmt.value());

        if (stmt.value()->type.has_value())
            scope->type = stmt.value()->type.value();
    }

    try_consume_err(TokenType::RIGHT_CURLY_BRACKET);

    return scope;
}

std::optional<Node::ScopeStmt *> Parser::parse_scope_stmt()
{
    if (!peek().has_value())
        return {};

    // RETURN ?
    if (peek_type(TokenType::RETURN))
    {
        consume();
        Node::StmtReturn *ret = allocator.emplace<Node::StmtReturn>();

        if (auto ne = parse_expr())
            ret->expr = ne.value();
        else
            exit_with("return value");

        Node::ScopeStmt *stmt = allocator.emplace<Node::ScopeStmt>(ret);
        stmt->type = ret->expr->type;

        return stmt;
    }

    // VAR IDENT = ?
    if (peek_type(TokenType::VAR) && peek_type(TokenType::IDENTIFIER, 1))
    {
        if (peek_type(TokenType::EQUAL, 2))
        {
            consume(); // var

            Node::StmtImplicitVar *var = allocator.emplace<Node::StmtImplicitVar>();
            var->identifier = consume();

            if (identifiers.contains(var->identifier.val.value()))
                exit_with("'" + var->identifier.val.value() + "' already used", "identifier");

            consume(); // =

            if (auto e = parse_expr())
            {
                var->expr = e.value();
            }
            else
            {
                exit_with("expression");
            }

            identifiers[var->identifier.val.value()] = var->expr->type;

            Node::ScopeStmt *stmt = allocator.emplace<Node::ScopeStmt>(var);
            return stmt;
        }
        else if (peek_type(TokenType::COLON, 2) && peek_type(TokenType::EQUAL, 4))
        {
            consume(); // var

            Node::StmtImplicitVar *var = allocator.emplace<Node::StmtImplicitVar>();
            var->identifier = consume();

            if (identifiers.contains(var->identifier.val.value()))
                exit_with("'" + var->identifier.val.value() + "' already used", "identifier");

            consume(); // :
            Token type = consume();
            consume(); // =

            if (auto e = parse_expr())
            {
                var->expr = e.value();
            }
            else
            {
                exit_with("expression");
            }

            if (var->expr->type != to_variable_type(type.type))
                exit_with(to_string(type.type), "variable type must be");

            identifiers[var->identifier.val.value()] = var->expr->type;

            Node::ScopeStmt *stmt = allocator.emplace<Node::ScopeStmt>(var);
            return stmt;
        }
        else if (peek_type(TokenType::COLON, 2))
        {
            consume(); // var
            Node::StmtExplicitVar *var = allocator.emplace<Node::StmtExplicitVar>();
            var->ident = consume();

            if (identifiers.contains(var->ident.val.value()))
                exit_with("'" + var->ident.val.value() + "' already used", "identifier");

            consume(); // :

            if (auto t = parse_type())
            {
                var->type = t.value();
            }
            else
            {
                exit_with("type");
            }

            identifiers[var->ident.val.value()] = var->type;

            Node::ScopeStmt *stmt = allocator.emplace<Node::ScopeStmt>(var);
            return stmt;
        }
        else
            exit_with("type declaration");
    }

    // IDENT = ?
    if (peek_type(TokenType::IDENTIFIER) && peek_type(TokenType::EQUAL, 1))
    {
        auto var_assign = allocator.alloc<Node::StmtVarAssign>();
        var_assign->ident = consume();

        if (!identifiers.contains(var_assign->ident.val.value()))
        {
            exit_with("'" + var_assign->ident.val.value() + "'", "unknown identifier");
        }

        consume(); // = token

        if (const auto expr = parse_expr())
        {
            var_assign->expr = expr.value();
        }
        else
            exit_with("expression");

        if (identifiers[var_assign->ident.val.value()] != var_assign->expr->type)
        {
            exit_with(to_string(var_assign->expr->type), "wrong type ");
        }

        return allocator.emplace<Node::ScopeStmt>(var_assign);
    }

    // IDENT( ? )
    if (peek_type(TokenType::IDENTIFIER) && peek_type(TokenType::LEFT_PARENTHESIS, 1))
    {
        auto fcall = allocator.alloc<Node::FuncCall>();
        fcall->ident = consume();

        if (is_buildin_func(fcall->ident.val.value()))
        {
            fcall->type = buildin_func_type.at(fcall->ident.val.value());
        }
        else if (const auto t = var_type(fcall->ident.val.value()))
        {
            fcall->type = t.value();
        }
        else
            exit_with(fcall->ident.val.value(), "unknown identifier ");

        consume(); // ( token

        fcall->args = parse_args();

        try_consume_err(TokenType::RIGHT_PARENTHESIS);

        return allocator.emplace<Node::ScopeStmt>(fcall);
    }

    // { ? }
    if (peek_type(TokenType::LEFT_CURLY_BACKET))
    {
        if (auto scope = parse_scope())
        {
            auto stmt = allocator.emplace<Node::ScopeStmt>(scope.value());
            return stmt;
        }
        else
            exit_with("scope");
    }

    // IF ( ? ) { ? } ?
    if (auto tif = try_consume(TokenType::IF))
    {
        try_consume_err(TokenType::LEFT_PARENTHESIS);

        auto stmt_if = allocator.emplace<Node::StmtIf>();

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

        auto stmt = allocator.emplace<Node::ScopeStmt>(stmt_if);
        return stmt;
    }

    return {};
}

std::vector<Node::Expr *> Parser::parse_args()
{
    std::vector<Node::Expr *> args{};

    if (const auto e = parse_expr())
    {
        args.push_back(e.value());

        while (const auto comma = try_consume(TokenType::COMMA))
        {
            if (const auto e = parse_expr())
            {
                args.push_back(e.value());
            }
            else
                exit_with("expression");
        }
    }

    return args;
}

std::optional<Node::IfPred *> Parser::parse_if_pred()
{
    if (auto t = try_consume(TokenType::ELIF))
    {
        try_consume_err(TokenType::LEFT_PARENTHESIS);
        auto elif_pred = allocator.alloc<Node::IfPredElif>();
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

        return allocator.emplace<Node::IfPred>(elif_pred);
    }

    if (try_consume(TokenType::ELSE))
    {
        auto else_pred = allocator.alloc<Node::IfPredElse>();
        if (const auto scope = parse_scope())
            else_pred->scope = scope.value();
        else
            exit_with("scope");

        return allocator.emplace<Node::IfPred>(else_pred);
    }

    return {};
}

std::optional<Node::Expr *> Parser::parse_expr(int min_prec)
{
    std::optional<Node::Term *> lterm = parse_term();
    if (!lterm.has_value())
        return {};

    auto expr = allocator.emplace<Node::Expr>(lterm.value());
    expr->type = lterm.value()->type;

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

        if (expr->type != VarType::VOID &&
            expr_rside.value()->type != VarType::VOID &&
            expr->type != expr_rside.value()->type)
        {
            exit_with(
                to_string(expr->type) + " " + to_string(op.type) + " " + to_string(expr_rside.value()->type),
                "wrong operation :");
        }

        auto bin_expr = allocator.emplace<Node::BinExpr>();
        auto expr_lside = allocator.emplace<Node::Expr>(expr->var);

        if (op.type == TokenType::PLUS)
        {
            auto add = allocator.emplace<Node::BinExprAdd>(expr_lside, expr_rside.value());
            bin_expr->var = add;
        }
        else if (op.type == TokenType::MINUS)
        {
            /// TODO: make sure you cannot sub strings
            auto sub = allocator.emplace<Node::BinExprSub>(expr_lside, expr_rside.value());
            bin_expr->var = sub;
        }
        else if (op.type == TokenType::STAR)
        {
            auto multi = allocator.emplace<Node::BinExprMulti>(expr_lside, expr_rside.value());
            bin_expr->var = multi;
        }
        else if (op.type == TokenType::SLASH)
        {
            /// TODO: make sure you cannot divide strings
            auto div = allocator.emplace<Node::BinExprDiv>(expr_lside, expr_rside.value());
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
    // FUNC CALL
    if (peek_type(TokenType::IDENTIFIER) && peek_type(TokenType::LEFT_PARENTHESIS, 1))
    {
        auto fcall = allocator.alloc<Node::FuncCall>();
        fcall->ident = consume();

        if (is_buildin_func(fcall->ident.val.value()))
        {
            fcall->type = buildin_func_type.at(fcall->ident.val.value());
        }
        else if (const auto t = var_type(fcall->ident.val.value()))
        {
            fcall->type = t.value();
        }
        else
            exit_with(fcall->ident.val.value(), "unknown identifier");

        consume(); // ( token

        fcall->args = parse_args();

        try_consume_err(TokenType::RIGHT_PARENTHESIS);

        auto term = allocator.emplace<Node::Term>(fcall);
        term->type = fcall->type;

        return term;
    }

    // VAR CALLS
    if (auto ident = try_consume(TokenType::IDENTIFIER))
    {
        auto expr_ident = allocator.emplace<Node::TermIdentifier>(ident.value());
        auto term = allocator.emplace<Node::Term>(expr_ident);

        if (const auto t = var_type(ident.value().val.value()))
        {
            term->type = t.value();
        }
        else
            exit_with(ident.value().val.value(), "unknown identifier");

        return term;
    }

    // LITERALS
    if (auto int_lit = try_consume(TokenType::INTEGER_LITERAL))
    {
        auto term_int_lit = allocator.emplace<Node::TermIntegerLiteral>(int_lit.value());
        auto term = allocator.emplace<Node::Term>(term_int_lit);
        term->type = VarType::INT;
        return term;
    }

    if (auto char_lit = try_consume(TokenType::CHAR_LITERAL))
    {
        auto term_char_lit = allocator.emplace<Node::TermCharLiteral>(char_lit.value());
        auto term = allocator.emplace<Node::Term>(term_char_lit);
        term->type = VarType::CHAR;
        return term;
    }

    // IN PARENTHESIS
    if (auto open_paren = try_consume(TokenType::LEFT_PARENTHESIS))
    {
        auto expr = parse_expr();
        if (!expr.has_value())
            exit_with("expression");

        try_consume_err(TokenType::RIGHT_PARENTHESIS);

        auto term_paren = allocator.emplace<Node::TermParen>(expr.value());
        auto term = allocator.emplace<Node::Term>(term_paren);
        term->type = expr.value()->type;
        return term;
    }

    return {};
}

std::optional<VarType> Parser::parse_type()
{
    if (auto t = try_consume(TokenType::TYPE_INT))
        return VarType::INT;

    if (auto t = try_consume(TokenType::TYPE_CHAR))
        return VarType::CHAR;

    return {};
}

#pragma once

#include <cassert>
#include <variant>

#include "tokenization.hpp"
#include "arena.hpp"

namespace Node
{
    struct Expr;

    struct TermIntegerLiteral
    {
        Token int_lit;
    };

    struct TermIdentifier
    {
        Token ident;
    };

    struct TermParen
    {
        Expr *expr;
    };

    struct Term
    {
        std::variant<TermIntegerLiteral *, TermIdentifier *, TermParen *> var;
    };

    struct BinExprAdd
    {
        Expr *lside;
        Expr *rside;
    };

    struct BinExprSub
    {
        Expr *lside;
        Expr *rside;
    };

    struct BinExprMulti
    {
        Expr *lside;
        Expr *rside;
    };

    struct BinExprDiv
    {
        Expr *lside;
        Expr *rside;
    };

    struct BinExpr
    {
        std::variant<BinExprAdd *, BinExprSub *, BinExprMulti *, BinExprDiv *> var;
    };

    struct Expr
    {
        std::variant<Term *, BinExpr *> var;
    };

    struct Stmt;

    struct Scope
    {
        std::vector<Stmt *> stmts;
    };

    struct StmtReturn
    {
        Expr *expr;
    };

    struct StmtLet
    {
        Token identifier;
        Expr *expr;
    };

    struct StmtIf
    {
        Expr* expr;
        Scope* scope;
    };

    struct Stmt
    {
        std::variant<StmtReturn *, StmtLet *, Scope *, StmtIf *> var;
    };

    struct Prog
    {
        std::vector<Stmt *> stmts;
    };
}

class Parser
{
private:
    const std::vector<Token> _tokens;

    size_t _index = 0;

    ArenaAllocator _allocator;

    std::optional<Token> peek(int offset = 0) const
    {
        if (_index + offset >= _tokens.size())
            return {};
        return _tokens[_index + offset];
    }

    bool peek_type(TokenType type, int offset = 0) const
    {
        return peek(offset).has_value() && peek(offset).value().type == type;
    }

    Token consume()
    {
        return _tokens[_index++];
    }

    Token try_consume(TokenType type, const std::string &err_msg)
    {
        if (peek().has_value() && peek().value().type == type)
        {
            return consume();
        }
        else
        {
            exit_with(err_msg);
            return {}; // unreachable
        }
    }

    std::optional<Token> try_consume(TokenType type)
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

public:
    Parser(std::vector<Token> tokens) : _tokens(std::move(tokens)), _allocator(1024 * 1024 * 4) {} // 4mb

    std::optional<Node::Term *> parse_term()
    {

        if (auto int_lit = try_consume(TokenType::INTEGER_LITERAL))
        {
            auto term_int_lit = _allocator.emplace<Node::TermIntegerLiteral>(int_lit.value());
            auto term = _allocator.emplace<Node::Term>(term_int_lit);
            return term;
        }

        if (auto ident = try_consume(TokenType::IDENTIFIER))
        {
            auto expr_ident = _allocator.emplace<Node::TermIdentifier>(ident.value());
            auto term = _allocator.emplace<Node::Term>(expr_ident);
            return term;
        }

        if (auto open_paren = try_consume(TokenType::LEFT_PARENTHESIS))
        {
            auto expr = parse_expr();
            if (!expr.has_value())
                exit_with("expected expression");

            try_consume(TokenType::RIGHT_PARENTHESIS, "expected `)`");

            auto term_paren = _allocator.emplace<Node::TermParen>(expr.value());
            auto term = _allocator.emplace<Node::Term>(term_paren);
            return term;
        }

        return {};
    }

    std::optional<Node::Expr *> parse_expr(int min_prec = 0)
    {
        std::optional<Node::Term *> lterm = parse_term();
        if (!lterm.has_value())
            return {};

        auto expr = _allocator.emplace<Node::Expr>(lterm.value());

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
                exit_with("unable to parse expression");

            auto bin_expr = _allocator.emplace<Node::BinExpr>();
            auto expr_lside = _allocator.emplace<Node::Expr>(expr->var);

            if (op.type == TokenType::PLUS)
            {
                auto add = _allocator.emplace<Node::BinExprAdd>(expr_lside, expr_rside.value());
                bin_expr->var = add;
            }
            else if (op.type == TokenType::MINUS)
            {
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
                auto div = _allocator.emplace<Node::BinExprDiv>(expr_lside, expr_rside.value());
                bin_expr->var = div;
            }
            else
                assert(false); // unreachable

            expr->var = bin_expr;
        }

        return expr;
    }

    std::optional<Node::Scope *> parse_scope()
    {
        if (!try_consume(TokenType::LEFT_CURLY_BACKET).has_value())
            return {};

        auto scope = _allocator.emplace<Node::Scope>();
        while (auto stmt = parse_stmt())
        {
            scope->stmts.push_back(stmt.value());
        }

        try_consume(TokenType::RIGHT_CURLY_BRACKET, "expected `}`");

        return scope;
    }

    std::optional<Node::Stmt *> parse_stmt()
    {
        if (!peek().has_value())
            return {};

        if (peek_type(TokenType::RETURN))
        {
            consume();
            Node::StmtReturn *ret = _allocator.emplace<Node::StmtReturn>();

            if (auto ne = parse_expr())
                ret->expr = ne.value();
            else
                exit_with("parsing error");

            Node::Stmt *stmt = _allocator.emplace<Node::Stmt>(ret);

            return stmt;
        }

        if (peek_type(TokenType::LET) &&
            peek_type(TokenType::IDENTIFIER, 1) &&
            peek_type(TokenType::EQUAL, 2))
        {
            consume();
            Node::StmtLet *let = _allocator.emplace<Node::StmtLet>();
            let->identifier = consume();
            consume();
            if (auto e = parse_expr())
            {
                let->expr = e.value();
            }
            else
            {
                exit_with("invalid expression");
            }

            Node::Stmt *stmt = _allocator.emplace<Node::Stmt>(let);
            return stmt;
        }

        if (peek_type(TokenType::LEFT_CURLY_BACKET))
        {
            if (auto scope = parse_scope())
            {
                auto stmt = _allocator.emplace<Node::Stmt>(scope.value());
                return stmt;
            }
            else exit_with("invalid scope");
        }

        if (auto tif = try_consume(TokenType::IF))
        {
            try_consume(TokenType::LEFT_PARENTHESIS, "expected `(`");

            auto stmt_if = _allocator.emplace<Node::StmtIf>();

            if (auto expr = parse_expr()) {
                stmt_if->expr = expr.value();
            }
            else exit_with("invalid expression");

            try_consume(TokenType::RIGHT_PARENTHESIS, "expected `)`");

            if (auto scope = parse_scope())
            {
                stmt_if->scope = scope.value();
            }
            else exit_with("missing scope");

            auto stmt = _allocator.emplace<Node::Stmt>(stmt_if);
            return stmt;
        }

        return {};
    }

    std::optional<Node::Prog> parse_prog()
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
                exit_with("invalid statement");
            }
        }

        return prog;
    };
};

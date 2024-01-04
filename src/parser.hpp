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
        Expr* expr;
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

    struct StmtReturn
    {
        Expr *expr;
    };

    struct StmtLet
    {
        Token identifier;
        Expr *expr;
    };

    struct Stmt
    {
        std::variant<StmtReturn *, StmtLet *> var;
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
            Node::TermIntegerLiteral *term_int_lit = _allocator.alloc<Node::TermIntegerLiteral>();
            term_int_lit->int_lit = int_lit.value();

            Node::Term *term = _allocator.alloc<Node::Term>();
            term->var = term_int_lit;

            return term;
        }

        if (auto ident = try_consume(TokenType::IDENTIFIER))
        {
            Node::TermIdentifier *term_ident = _allocator.alloc<Node::TermIdentifier>();
            term_ident->ident = ident.value();

            Node::Term *term = _allocator.alloc<Node::Term>();
            term->var = term_ident;

            return term;
        }

        if (auto open_paren = try_consume(TokenType::LEFT_PARENTHESIS))
        {
            auto expr = parse_expr();
            if (!expr.has_value())
                exit_with("expected expression");
            
            try_consume(TokenType::RIGHT_PARENTHESIS, "expected `)`");

            auto paren = _allocator.alloc<Node::TermParen>();
            paren->expr = expr.value();

            auto term = _allocator.alloc<Node::Term>();
            term->var = paren;
            return term;    
        }

        return {};
    }

    std::optional<Node::Expr *> parse_expr(int min_prec = 0)
    {
        std::optional<Node::Term *> lterm = parse_term();
        if (!lterm.has_value())
            return {};

        auto expr = _allocator.alloc<Node::Expr>();
        expr->var = lterm.value();

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

            auto bin_expr = _allocator.alloc<Node::BinExpr>();
            auto expr_lside = _allocator.alloc<Node::Expr>();
            expr_lside->var = expr->var;
            if (op.type == TokenType::PLUS)
            {
                auto add = _allocator.alloc<Node::BinExprAdd>();
                add->lside = expr_lside;
                add->rside = expr_rside.value();
                bin_expr->var = add;
            }
            else if (op.type == TokenType::MINUS)
            {
                auto minus = _allocator.alloc<Node::BinExprSub>();
                minus->lside = expr_lside;
                minus->rside = expr_rside.value();
                bin_expr->var = minus;
            }
            else if (op.type == TokenType::STAR)
            {
                auto multi = _allocator.alloc<Node::BinExprMulti>();
                multi->lside = expr_lside;
                multi->rside = expr_rside.value();
                bin_expr->var = multi;
            }
            else if (op.type == TokenType::SLASH)
            {
                auto div = _allocator.alloc<Node::BinExprDiv>();
                div->lside = expr_lside;
                div->rside = expr_rside.value();
                bin_expr->var = div;
            }
            else
                assert(false); // unreachable

            expr->var = bin_expr;
        }

        return expr;
    }

    std::optional<Node::Stmt *> parse_stmt()
    {
        if (!peek().has_value())
            return {};

        if (peek().value().type == TokenType::RETURN)
        {
            consume();
            Node::StmtReturn *s = _allocator.alloc<Node::StmtReturn>();

            if (auto ne = parse_expr())
                s->expr = ne.value();
            else exit_with("parsing error");

            Node::Stmt *stmt = _allocator.alloc<Node::Stmt>();
            stmt->var = s;

            return stmt;
        }

        if (peek().value().type == TokenType::LET &&
            peek(1).has_value() && peek(1).value().type == TokenType::IDENTIFIER &&
            peek(2).has_value() && peek(2).value().type == TokenType::EQUAL)
        {
            consume();
            Node::StmtLet *let = _allocator.alloc<Node::StmtLet>();
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

            Node::Stmt *stmt = _allocator.alloc<Node::Stmt>();
            stmt->var = let;

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

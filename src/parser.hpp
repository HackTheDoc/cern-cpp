#pragma once

#include <variant>

#include "tokenization.hpp"
#include "arena.hpp"

namespace Node
{
    struct TermIntegerLiteral
    {
        Token int_lit;
    };

    struct TermIdentifier
    {
        Token ident;
    };


    struct Term
    {
        std::variant<TermIntegerLiteral *, TermIdentifier *> var;
    };

    struct Expr;

    struct BinExprAdd
    {
        Expr *lside;
        Expr *rside;
    };

    struct BinExprMulti
    {
        Expr *lside;
        Expr *rside;
    };

    struct BinExpr
    {
        std::variant<BinExprAdd *, BinExprMulti *> var;
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

    Token try_consume(TokenType type, const std::string& err_msg)
    {
        if (peek().has_value() && peek().value().type == type)
        {
            return consume();
        }
        else
        {
            std::cerr << err_msg << std::endl;
            exit(EXIT_FAILURE);
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

    std::optional<Node::Term*> parse_term()
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

        return {};
    }

    std::optional<Node::Expr *> parse_expr()
    {
        if (!peek().has_value())
            return {};

        if (std::optional<Node::Term *> term = parse_term())
        {
            if (try_consume(TokenType::PLUS).has_value())
            {

                auto bin_expr = _allocator.alloc<Node::BinExpr>();
                auto bin_expr_add = _allocator.alloc<Node::BinExprAdd>();

                auto lside = _allocator.alloc<Node::Expr>();
                lside->var = term.value();
                bin_expr_add->lside = lside;

                if (auto rside = parse_expr())
                {
                    bin_expr_add->rside = rside.value();
                }
                else
                {
                    std::cerr << "missing expression" << std::endl;
                    exit(EXIT_FAILURE);
                }

                bin_expr->var = bin_expr_add;

                auto expr = _allocator.alloc<Node::Expr>();
                expr->var = bin_expr;
                return expr;
            }
            if (try_consume(TokenType::STAR).has_value())
            {

                auto bin_expr = _allocator.alloc<Node::BinExpr>();
                auto bin_expr_multi = _allocator.alloc<Node::BinExprMulti>();

                auto lside = _allocator.alloc<Node::Expr>();
                lside->var = term.value();
                bin_expr_multi->lside = lside;

                if (auto rside = parse_expr())
                {
                    bin_expr_multi->rside = rside.value();
                }
                else
                {
                    std::cerr << "missing expression" << std::endl;
                    exit(EXIT_FAILURE);
                }

                bin_expr->var = bin_expr_multi;

                auto expr = _allocator.alloc<Node::Expr>();
                expr->var = bin_expr;
                return expr;
            }
            else
            {
                auto expr = _allocator.alloc<Node::Expr>();
                expr->var = term.value();
                return expr;
            }
            
        }

        return {};
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
            else
            {
                std::cerr << "parsing error" << std::endl;
                exit(EXIT_FAILURE);
            }

            Node::Stmt *stmt = _allocator.alloc<Node::Stmt>();
            stmt->var = s;

            return stmt;
        }

        if (peek().value().type == TokenType::LET &&
            peek(1).has_value() && peek(1).value().type == TokenType::IDENTIFIER &&
            peek(2).has_value() && peek(2).value().type == TokenType::EQUAL)
        {
            consume();
            Node::StmtLet *s = _allocator.alloc<Node::StmtLet>();
            s->identifier = consume();
            consume();
            if (auto e = parse_expr())
            {
                s->expr = e.value();
            }
            else
            {
                std::cerr << "invalid expression" << std::endl;
                exit(EXIT_FAILURE);
            }

            Node::Stmt *stmt = _allocator.alloc<Node::Stmt>();
            stmt->var = s;

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
                std::cerr << "invalid statement" << std::endl;
                exit(EXIT_FAILURE);
            }
        }

        return prog;
    };
};

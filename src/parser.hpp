#pragma once

#include <variant>

#include "tokenization.hpp"

namespace Node
{
    struct ExprIntegerLiteral 
    {
        Token int_lit;
    };

    struct ExprIdentifier
    {
        Token ident;
    };

    struct Expr {
        std::variant<ExprIntegerLiteral, ExprIdentifier> var;
    };

    struct StmtReturn
    {
        Expr expr;
    };

    struct StmtLet
    {
        Token identifier;
        Expr expr;
    };

    struct Stmt
    {
        std::variant<StmtReturn, StmtLet> var;
    };

    struct Prog
    {
        std::vector<Stmt> stmts;
    };
}

class Parser
{
private:
    const std::vector<Token> _tokens;

    size_t _index = 0;

    std::optional<Token> peek(int offset = 0) const
    {
        if (_index + offset >= _tokens.size())
            return {};
        return _tokens[_index+offset];
    }

    Token consume()
    {
        return _tokens[_index++];
    }

public:
    Parser(std::vector<Token> tokens) : _tokens(std::move(tokens)) {}

    std::optional<Node::Expr> parse_expr()
    {
        if (!peek().has_value())
            return {};

        if (peek().value().type == TokenType::INTEGER_LITERAL)
            return Node::Expr{.var = Node::ExprIntegerLiteral{.int_lit = consume()}};

        if (peek().value().type == TokenType::IDENTIFIER)
            return Node::Expr{.var = Node::ExprIdentifier{.ident = consume()}};

        return {};
    }

    std::optional<Node::Stmt> parse_stmt() {
        if (!peek().has_value())
            return {};

        if (peek().value().type == TokenType::RETURN)
        {
            consume();
            Node::StmtReturn s;

            if (auto ne = parse_expr())
                s = Node::StmtReturn{.expr = ne.value()};
            else
            {
                std::cerr << "parsing error" << std::endl;
                exit(EXIT_FAILURE);
            }

            return Node::Stmt{.var = s};
        }

        if (peek().value().type == TokenType::LET &&
            peek(1).has_value() && peek(1).value().type == TokenType::IDENTIFIER &&
            peek(2).has_value() && peek(2).value().type == TokenType::EQUAL)
        {
            consume();
            Node::StmtLet s = Node::StmtLet{.identifier = consume()};
            consume();
            if (auto e = parse_expr()) {
                s.expr = e.value();
            }
            else {
                std::cerr << "invalid expression" << std::endl;
                exit(EXIT_FAILURE);
            }

            return Node::Stmt{.var = s};
        }

        return {};
    }

    std::optional<Node::Prog> parse_prog() {
        Node::Prog prog;
        
        while (peek().has_value()) {
            if (auto stmt = parse_stmt()) {
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

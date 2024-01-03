#pragma once

#include "tokenization.hpp"

namespace Node
{
    struct Expr
    {
        Token val;
    };

    struct Return
    {
        Expr expr;
    };
}

class Parser
{
private:
    const std::vector<Token> _tokens;

    size_t _index = 0;

    std::optional<Token> peek(int ahead = 1) const
    {
        if (_index + ahead > _tokens.size())
            return {};
        return _tokens[_index];
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
        {
            return Node::Expr{.val = consume()};
        }

        return {};
    }

    std::optional<Node::Return> parse()
    {
        std::optional<Node::Return> nr;

        while (peek().has_value())
        {
            if (peek().value().type == TokenType::RETURN)
            {
                consume();
                if (auto ne = parse_expr())
                {
                    nr = Node::Return{.expr = ne.value()};
                }
                else
                {
                    std::cerr << "parsing error" << std::endl;
                    exit(EXIT_FAILURE);
                }
            }
        };

        _index = 0;
        return nr;
    }
};

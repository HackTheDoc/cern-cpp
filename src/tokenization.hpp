#pragma once

#include <iostream>
#include <optional>
#include <vector>
#include <string>

void exit_with(const std::string &err_msg)
{
    std::cerr << err_msg << std::endl;
    exit(EXIT_FAILURE);
}

enum TokenType
{
    RETURN,
    INTEGER_LITERAL,
    LET,
    IDENTIFIER,

    IF,
    ELIF,
    ELSE,

    EQUAL,
    LEFT_PARENTHESIS,
    RIGHT_PARENTHESIS,
    LEFT_CURLY_BACKET,
    RIGHT_CURLY_BRACKET,
    PLUS,
    MINUS,
    STAR,
    SLASH
};

std::optional<int> bin_prec(TokenType type)
{
    switch (type)
    {
    case TokenType::PLUS:
    case TokenType::MINUS:
        return 0;
    case TokenType::STAR:
    case TokenType::SLASH:
        return 1;
    default:
        return {};
    }
}

struct Token
{
    TokenType type;
    std::optional<std::string> val{};
};

class Tokenizer
{
private:
    const std::string _src;
    size_t _index = 0;

    std::optional<char> peek(int offset = 0) const
    {
        if (_index + offset >= _src.length())
            return {};
        return _src.at(_index + offset);
    }

    char consume()
    {
        return _src[_index++];
    }

public:
    Tokenizer(const std::string &src) : _src(std::move(src)) {}

    std::vector<Token> tokenize()
    {
        std::vector<Token> tokens;
        std::string buf;

        while (peek().has_value())
        {
            if (peek().value() == '/' && peek(1).has_value() && peek(1).value() == '/')
            {
                while (peek().has_value() && peek().value() != '\n')
                    consume();
            }
            else if (peek().value() == '/' && peek(1).has_value() && peek(1).value() == '*')
            {
                consume();
                consume();
                while (peek().has_value() && peek().value() != '*' && peek(1).has_value() && peek(1).value() != '/')
                    consume();

                if (peek().has_value())
                    consume();
                if (peek().has_value())
                    consume();
            }
            else if (std::isalpha(peek().value()))
            {
                buf.push_back(consume());
                while (peek().has_value() && std::isalnum(peek().value()))
                {
                    buf.push_back(consume());
                }

                if (buf == "return")
                    tokens.push_back({.type = TokenType::RETURN});
                else if (buf == "let")
                    tokens.push_back({.type = TokenType::LET});
                else if (buf == "if")
                    tokens.push_back({.type = TokenType::IF});
                else if (buf == "elif")
                    tokens.push_back({.type = TokenType::ELIF});
                else if (buf == "else")
                    tokens.push_back({.type = TokenType::ELSE});
                else
                    tokens.push_back({.type = TokenType::IDENTIFIER, .val = buf});

                buf.clear();
            }
            else if (std::isdigit(peek().value()))
            {
                buf.push_back(consume());
                while (peek().has_value() && std::isdigit(peek().value()))
                {
                    buf.push_back(consume());
                }
                tokens.push_back({.type = TokenType::INTEGER_LITERAL, .val = buf});
                buf.clear();
            }
            else if (peek().value() == '=')
            {
                consume();
                tokens.push_back({.type = TokenType::EQUAL});
            }
            else if (peek().value() == '(')
            {
                consume();
                tokens.push_back({.type = TokenType::LEFT_PARENTHESIS});
            }
            else if (peek().value() == ')')
            {
                consume();
                tokens.push_back({.type = TokenType::RIGHT_PARENTHESIS});
            }
            else if (peek().value() == '{')
            {
                consume();
                tokens.push_back({.type = TokenType::LEFT_CURLY_BACKET});
            }
            else if (peek().value() == '}')
            {
                consume();
                tokens.push_back({.type = TokenType::RIGHT_CURLY_BRACKET});
            }
            else if (peek().value() == '+')
            {
                consume();
                tokens.push_back({.type = TokenType::PLUS});
            }
            else if (peek().value() == '-')
            {
                consume();
                tokens.push_back({.type = TokenType::MINUS});
            }
            else if (peek().value() == '*')
            {
                consume();
                tokens.push_back({.type = TokenType::STAR});
            }
            else if (peek().value() == '/')
            {
                consume();
                tokens.push_back({.type = TokenType::SLASH});
            }
            else if (std::isspace(peek().value()))
            {
                consume();
            }
            else if (peek().value() == '\n')
            {
                consume();
            }
            else
                exit_with("invalid token '" + peek().value() + '\'');
        }

        _index = 0;

        return tokens;
    }
};

#pragma once

#include <iostream>
#include <optional>
#include <vector>
#include <string>

enum TokenType
{
    RETURN,
    INTEGER_LITERAL
};

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

    std::optional<char> peek(int ahead = 1) const
    {
        if (_index + ahead > _src.length())
            return {};
        return _src[_index];
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
            if (std::isspace(peek().value()))
            {
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
                {
                    tokens.push_back({.type = TokenType::RETURN});
                    buf.clear();
                }
                else
                {
                    std::cerr << "syntax error" << std::endl;
                    exit(EXIT_FAILURE);
                }
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
            else
            {
                std::cerr << "syntax error" << std::endl;
                exit(EXIT_FAILURE);
            }
        }

        _index = 0;

        return tokens;
    }
};

#pragma once

#include <iostream>
#include <optional>
#include <vector>
#include <string>

enum TokenType
{
    RETURN,
    INTEGER_LITERAL,
    LET,
    IDENTIFIER,
    EQUAL,
    LEFT_PARENTHESIS,
    RIGHT_PARENTHESIS
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

    std::optional<char> peek(int offset = 0) const
    {
        if (_index + offset >= _src.length())
            return {};
        return _src[_index+offset];
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
            if (std::isalpha(peek().value()))
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
            else if (peek().value() == '=') {
                consume();
                tokens.push_back({.type = TokenType::EQUAL});
            }
            else if (peek().value() == '(') {
                consume();
                tokens.push_back({.type = TokenType::LEFT_PARENTHESIS});
            }
            else if (peek().value() == ')') {
                consume();
                tokens.push_back({.type = TokenType::RIGHT_PARENTHESIS});
            }
            else if (std::isspace(peek().value())) {
                consume();
            }
            else if (peek().value() == '\n') {
                consume();
            }
            else
            {   
                std::cerr << "syntax error : " << peek().value() << std::endl;
                exit(EXIT_FAILURE);
            }
        }

        _index = 0;

        return tokens;
    }
};

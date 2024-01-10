#pragma once

#include <iostream>
#include <optional>
#include <vector>
#include <string>

enum TokenType
{
    RETURN,
    VAR,
    IDENTIFIER,
    TYPE_INT,
    INTEGER_LITERAL,

    IF,
    ELIF,
    ELSE,

    EQUAL,
    COLON,
    LEFT_PARENTHESIS,
    RIGHT_PARENTHESIS,
    LEFT_CURLY_BACKET,
    RIGHT_CURLY_BRACKET,
    PLUS,
    MINUS,
    STAR,
    SLASH
};

std::string to_string(const TokenType type)
{
    switch (type)
    {
    case TokenType::RETURN:
        return "return value";
    case TokenType::VAR:
        return "`var`";
    case TokenType::IDENTIFIER:
        return "identifier";
    case TokenType::TYPE_INT:
        return "int";
    case TokenType::INTEGER_LITERAL:
        return "integer literal";
    case TokenType::IF:
        return "`if`";
    case TokenType::ELIF:
        return "`elif`";
    case TokenType::ELSE:
        return "`else`";
    case TokenType::EQUAL:
        return "`=`";
    case TokenType::COLON:
        return "`:`";
    case TokenType::LEFT_PARENTHESIS:
        return "`(`";
    case TokenType::RIGHT_PARENTHESIS:
        return "`)`";
    case TokenType::LEFT_CURLY_BACKET:
        return "`{`";
    case TokenType::RIGHT_CURLY_BRACKET:
        return "`}`";
    case TokenType::PLUS:
        return "`+`";
    case TokenType::MINUS:
        return "`-`";
    case TokenType::STAR:
        return "`*`";
    case TokenType::SLASH:
        return "`/`";
    default:
        return "";
    }
}

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
    int line;
    std::optional<std::string> val{};
};

class Tokenizer
{
private:
    const std::string _src;
    size_t _index = 0;

    std::optional<char> peek(const size_t offset = 0) const
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
        int line_count = 1;

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
                /*
                while (peek().has_value() && peek().value() != '*' && peek(1).has_value() && peek(1).value() != '/')
                {
                    consume();
                }
                */
                while (peek().has_value())
                {
                    if (peek().value() == '*' && peek(1).has_value() && peek(1).value() == '/')
                        break;
                    consume();
                }

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
                    tokens.push_back({.type = TokenType::RETURN, .line = line_count});
                else if (buf == "var")
                    tokens.push_back({.type = TokenType::VAR, .line = line_count});
                else if (buf == "int")
                    tokens.push_back({.type = TokenType::TYPE_INT, .line = line_count});
                else if (buf == "if")
                    tokens.push_back({.type = TokenType::IF, .line = line_count});
                else if (buf == "elif")
                    tokens.push_back({.type = TokenType::ELIF, .line = line_count});
                else if (buf == "else")
                    tokens.push_back({.type = TokenType::ELSE, .line = line_count});
                else
                    tokens.push_back({.type = TokenType::IDENTIFIER, .line = line_count, .val = buf});

                buf.clear();
            }
            else if (std::isdigit(peek().value()))
            {
                buf.push_back(consume());
                while (peek().has_value() && std::isdigit(peek().value()))
                {
                    buf.push_back(consume());
                }
                tokens.push_back({
                    .type = TokenType::INTEGER_LITERAL,
                    .line = line_count,
                    .val = buf
                });
                buf.clear();
            }
            else if (peek().value() == '=')
            {
                consume();
                tokens.push_back({.type = TokenType::EQUAL, .line = line_count});
            }
            else if (peek().value() == ':')
            {
                consume();
                tokens.push_back({.type = TokenType::COLON, .line = line_count});
            }
            else if (peek().value() == '(')
            {
                consume();
                tokens.push_back({.type = TokenType::LEFT_PARENTHESIS, .line = line_count});
            }
            else if (peek().value() == ')')
            {
                consume();
                tokens.push_back({.type = TokenType::RIGHT_PARENTHESIS, .line = line_count});
            }
            else if (peek().value() == '{')
            {
                consume();
                tokens.push_back({.type = TokenType::LEFT_CURLY_BACKET, .line = line_count});
            }
            else if (peek().value() == '}')
            {
                consume();
                tokens.push_back({.type = TokenType::RIGHT_CURLY_BRACKET, .line = line_count});
            }
            else if (peek().value() == '+')
            {
                consume();
                tokens.push_back({.type = TokenType::PLUS, .line = line_count});
            }
            else if (peek().value() == '-')
            {
                consume();
                tokens.push_back({.type = TokenType::MINUS, .line = line_count});
            }
            else if (peek().value() == '*')
            {
                consume();
                tokens.push_back({.type = TokenType::STAR, .line = line_count});
            }
            else if (peek().value() == '/')
            {
                consume();
                tokens.push_back({.type = TokenType::SLASH, .line = line_count});
            }
            else if (peek().value() == '\n')
            {
                line_count++;
                consume();
            }
            else if (std::isspace(peek().value()))
            {
                consume();
            }
            else
            {
                std::cout << "[Tokenization Error] invalid token `" << peek().value() << "` on line " << line_count << std::endl;
                exit(EXIT_FAILURE);
            }
        }

        _index = 0;

        return tokens;
    }
};

#pragma once

#include <iostream>
#include <optional>
#include <vector>
#include <string>

enum TokenType
{
    RETURN,
    VAR,
    FUNC,
    IDENTIFIER,

    TYPE_INT,
    TYPE_CHAR,
    
    INTEGER_LITERAL,
    CHAR_LITERAL,

    IF,
    ELIF,
    ELSE,

    EQUAL,
    COLON,
    COMMA,
    LEFT_PARENTHESIS,
    RIGHT_PARENTHESIS,
    LEFT_CURLY_BACKET,
    RIGHT_CURLY_BRACKET,
    PLUS,
    MINUS,
    STAR,
    SLASH
};

std::string to_string(const TokenType type);

std::optional<int> bin_prec(TokenType type);

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
    
    std::optional<char> peek(const size_t offset = 0) const;

    char consume();

public:
    Tokenizer(const std::string &src);
    
    std::vector<Token> tokenize();
};
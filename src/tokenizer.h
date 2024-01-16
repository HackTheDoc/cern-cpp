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

    TYPE_BOOL,
    TYPE_INT,
    TYPE_CHAR,

    BOOLEAN_LITEARL,
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
    SLASH,

    IS_EQUAL,
    IS_NOT_EQUAL,
    GREATER_OR_EQUAL,
    GREATER,
    LOWER_OR_EQUAL,
    LOWER,
    AND,
    OR,
    NOT
};

/// @brief basic conversion function
/// @param type type to convert
/// @return string equivalent to the given token type
std::string to_string(const TokenType type);

/// @brief calc the operator prec of a token
/// @param type
/// @return an optional value of the operator prec (return nothing if the token is not an operator)
std::optional<int> op_prec(TokenType type);

bool is_operator(TokenType type);

/// @brief a token is represented by its type, the line it is on and an optional value
struct Token
{
    TokenType type;
    int line;
    std::optional<std::string> val{};
};

class Tokenizer
{
private:
    /// @brief src string containing the code to tokenize
    const std::string _src;
    /// @brief index of the current character
    size_t _index = 0;

    /// @brief peek a character value (default: current)
    /// @param offset to peek forward or backward (default: 0)
    std::optional<char> peek(const size_t offset = 0) const;

    /// @brief consume the current char and move to the next one
    /// @return the consumed char
    char consume();

public:
    /// @brief Create a tokenizer
    /// @param src string containing the code to tokenize
    Tokenizer(const std::string &src);

    /// @brief start tokenization
    std::vector<Token> tokenize();
};
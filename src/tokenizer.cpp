#include "tokenizer.h"

std::string to_string(const TokenType type) {
    switch (type) {
    case TokenType::RETURN:
        return "return value";
    case TokenType::VAR:
        return "var";
    case TokenType::FUNC:
        return "func";
    case TokenType::IDENTIFIER:
        return "identifier";
    case TokenType::TYPE_BOOL:
        return "bool";
    case TokenType::TYPE_INT:
        return "int";
    case TokenType::TYPE_CHAR:
        return "char";
    case TokenType::TYPE_STRING:
        return "string";
    case TokenType::BOOLEAN_LITEARL:
        return "boolean literal";
    case TokenType::INTEGER_LITERAL:
        return "integer literal";
    case TokenType::CHAR_LITERAL:
        return "char literal";
    case TokenType::STRING_LITERAL:
        return "string literal";
    case TokenType::WHILE:
        return "while";
    case TokenType::IF:
        return "if";
    case TokenType::ELIF:
        return "elif";
    case TokenType::ELSE:
        return "else";
    case TokenType::EQUAL:
        return "=";
    case TokenType::COLON:
        return ":";
    case TokenType::COMMA:
        return ",";
    case TokenType::LEFT_PARENTHESIS:
        return "(";
    case TokenType::RIGHT_PARENTHESIS:
        return ")";
    case TokenType::LEFT_CURLY_BACKET:
        return "{";
    case TokenType::RIGHT_CURLY_BRACKET:
        return "}";
    case TokenType::PLUS:
        return "+";
    case TokenType::MINUS:
        return "-";
    case TokenType::STAR:
        return "*";
    case TokenType::SLASH:
        return "/";
    case TokenType::INCREMENTATOR:
        return "++";
    case TokenType::DECREMENTATOR:
        return "--";
    case TokenType::IS_EQUAL:
        return "==";
    case TokenType::IS_NOT_EQUAL:
        return "!=";
    case TokenType::GREATER_OR_EQUAL:
        return ">=";
    case TokenType::GREATER:
        return ">";
    case TokenType::LOWER_OR_EQUAL:
        return "<=";
    case TokenType::LOWER:
        return "<";
    case TokenType::AND:
        return "&&";
    case TokenType::OR:
        return "||";
    case TokenType::NOT:
        return "!";
    default:
        return "";
    }
}

std::optional<int> op_prec(TokenType type) {
    switch (type) {
    case TokenType::IS_EQUAL:
    case TokenType::IS_NOT_EQUAL:
    case TokenType::GREATER_OR_EQUAL:
    case TokenType::GREATER:
    case TokenType::LOWER_OR_EQUAL:
    case TokenType::LOWER:

    case TokenType::PLUS:
    case TokenType::MINUS:
    case TokenType::INCREMENTATOR:
    case TokenType::DECREMENTATOR:
        return 0;

    case TokenType::NOT:
    case TokenType::AND:
    case TokenType::OR:

    case TokenType::STAR:
    case TokenType::SLASH:
        return 1;
    default:
        return {};
    }
}

Tokenizer::Tokenizer(const std::string& src)
    : _src(std::move(src)) {
}

std::optional<char> Tokenizer::peek(const size_t offset) const {
    if (_index + offset >= _src.length())
        return {};
    return _src.at(_index + offset);
}

char Tokenizer::consume() {
    return _src[_index++];
}

std::vector<Token> Tokenizer::tokenize() {
    std::vector<Token> tokens;
    std::string buf;
    int line_count = 1;

    while (peek().has_value()) {
        if (peek().value() == '/' && peek(1).has_value() && peek(1).value() == '/') {
            while (peek().has_value() && peek().value() != '\n')
                consume();
        }
        else if (peek().value() == '/' && peek(1).has_value() && peek(1).value() == '*') {
            consume();
            consume();

            while (peek().has_value()) {
                if (peek().value() == '*' && peek(1).has_value() && peek(1).value() == '/')
                    break;
                if (peek().value() == '\n')
                    line_count++;
                consume();
            }

            if (peek().has_value())
                consume();
            if (peek().has_value())
                consume();
        }
        else if (std::isalpha(peek().value())) {
            buf.push_back(consume());
            while (peek().has_value() &&
                (std::isalnum(peek().value()) || peek().value() == '_')) {
                buf.push_back(consume());
            }

            // TYPES
            if (buf == "bool")
                tokens.push_back({ .type = TokenType::TYPE_BOOL, .line = line_count });
            else if (buf == "int")
                tokens.push_back({ .type = TokenType::TYPE_INT, .line = line_count });
            else if (buf == "char")
                tokens.push_back({ .type = TokenType::TYPE_CHAR, .line = line_count });
            else if (buf == "string")
                tokens.push_back({ .type = TokenType::TYPE_STRING, .line = line_count });

            // KEYWORDS
            else if (buf == "true")
                tokens.push_back({ .type = TokenType::BOOLEAN_LITEARL, .line = line_count, .val = "true" });
            else if (buf == "false")
                tokens.push_back({ .type = TokenType::BOOLEAN_LITEARL, .line = line_count, .val = "false" });
            else if (buf == "var")
                tokens.push_back({ .type = TokenType::VAR, .line = line_count });
            else if (buf == "func")
                tokens.push_back({ .type = TokenType::FUNC, .line = line_count });
            else if (buf == "return")
                tokens.push_back({ .type = TokenType::RETURN, .line = line_count });
            else if (buf == "while")
                tokens.push_back({ .type = TokenType::WHILE, .line = line_count });
            else if (buf == "if")
                tokens.push_back({ .type = TokenType::IF, .line = line_count });
            else if (buf == "elif")
                tokens.push_back({ .type = TokenType::ELIF, .line = line_count });
            else if (buf == "else")
                tokens.push_back({ .type = TokenType::ELSE, .line = line_count });
            else
                tokens.push_back({ .type = TokenType::IDENTIFIER, .line = line_count, .val = buf });

            buf.clear();
        }
        else if (std::isdigit(peek().value())) {
            buf.push_back(consume());
            while (peek().has_value() && std::isdigit(peek().value())) {
                buf.push_back(consume());
            }
            tokens.push_back({ .type = TokenType::INTEGER_LITERAL,
                              .line = line_count,
                              .val = buf });
            buf.clear();
        }
        else if (peek().value() == '=') {
            consume();

            if (peek().has_value() && peek().value() == '=') {
                consume();
                tokens.push_back({ .type = TokenType::IS_EQUAL, .line = line_count });
            }
            else
                tokens.push_back({ .type = TokenType::EQUAL, .line = line_count });
        }
        else if (peek().value() == ':') {
            consume();
            tokens.push_back({ .type = TokenType::COLON, .line = line_count });
        }
        else if (peek().value() == ',') {
            consume();
            tokens.push_back({ .type = TokenType::COMMA, .line = line_count });
        }
        else if (peek().value() == '(') {
            consume();
            tokens.push_back({ .type = TokenType::LEFT_PARENTHESIS, .line = line_count });
        }
        else if (peek().value() == ')') {
            consume();
            tokens.push_back({ .type = TokenType::RIGHT_PARENTHESIS, .line = line_count });
        }
        else if (peek().value() == '{') {
            consume();
            tokens.push_back({ .type = TokenType::LEFT_CURLY_BACKET, .line = line_count });
        }
        else if (peek().value() == '}') {
            consume();
            tokens.push_back({ .type = TokenType::RIGHT_CURLY_BRACKET, .line = line_count });
        }
        else if (peek().value() == '+') {
            consume();

            if (peek().has_value() && peek().value() == '+') {
                consume();
                tokens.push_back({ .type = TokenType::INCREMENTATOR, .line = line_count });
            }
            else
                tokens.push_back({ .type = TokenType::PLUS, .line = line_count });
        }
        else if (peek().value() == '-') {
            consume();

            if (peek().has_value() && peek().value() == '-') {
                consume();
                tokens.push_back({ .type = TokenType::DECREMENTATOR, .line = line_count });
            }
            else
                tokens.push_back({ .type = TokenType::MINUS, .line = line_count });
        }
        else if (peek().value() == '*') {
            consume();
            tokens.push_back({ .type = TokenType::STAR, .line = line_count });
        }
        else if (peek().value() == '/') {
            consume();
            tokens.push_back({ .type = TokenType::SLASH, .line = line_count });
        }
        else if (peek().value() == '!') {
            consume();

            if (peek().has_value() && peek().value() == '=') {
                consume();
                tokens.push_back({ .type = TokenType::IS_NOT_EQUAL, .line = line_count });
            }
            else
                tokens.push_back({ .type = TokenType::NOT, .line = line_count });
        }
        else if (peek().value() == '&') {
            consume();

            if (!peek().has_value() || peek().value() != '&') {
                std::cerr << "expected `&` on line " << line_count << std::endl;
                exit(EXIT_FAILURE);
            }
            tokens.push_back({ .type = TokenType::AND, .line = line_count });
        }
        else if (peek().value() == '|') {
            consume();

            if (!peek().has_value() || peek().value() != '|') {
                std::cerr << "expected `|` on line " << line_count << std::endl;
                exit(EXIT_FAILURE);
            }

            tokens.push_back({ .type = TokenType::OR, .line = line_count });
        }
        else if (peek().value() == '>') {
            consume();

            if (peek().has_value() && peek().value() == '=') {
                consume();
                tokens.push_back({ .type = TokenType::GREATER_OR_EQUAL, .line = line_count });
            }
            else
                tokens.push_back({ .type = TokenType::GREATER, .line = line_count });
        }
        else if (peek().value() == '<') {
            consume();

            if (peek().has_value() && peek().value() == '=') {
                consume();
                tokens.push_back({ .type = TokenType::LOWER_OR_EQUAL, .line = line_count });
            }
            else
                tokens.push_back({ .type = TokenType::LOWER, .line = line_count });
        }
        else if (peek().value() == '\'') {
            consume(); // '

            if (peek().has_value() && isalnum(peek().value())) {
                std::string c;
                c += consume();
                tokens.push_back({ .type = TokenType::CHAR_LITERAL, .line = line_count, .val = c });

                if (!peek().has_value() || peek().value() != '\'') {
                    std::cerr << "[Error] expected `'` on line " << line_count << std::endl;
                    exit(EXIT_FAILURE);
                }

                consume(); // '
            }
            else {
                std::cerr << "[Error] expected a valid char on line " << line_count << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        else if (peek().value() == '"') {
            consume(); // "

            while (peek().has_value() &&
                (std::isalnum(peek().value()) || peek().value() != '"')) {
                buf.push_back(consume());
            }

            tokens.push_back({ .type = TokenType::STRING_LITERAL, .line = line_count, .val = buf });
            buf.clear();
            if (!peek().has_value() || peek().value() != '"') {
                std::cerr << "[Error] expected `\"` on line " << line_count << std::endl;
                exit(EXIT_FAILURE);
            }

            consume(); // "
            line_count++;
        }
        else if (peek().value() == '\n') {
            line_count++;
            consume();
        }
        else if (std::isspace(peek().value())) {
            consume();
        }
        else {
            std::cerr << "[Error] invalid token `" << peek().value() << "` on line " << line_count << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    _index = 0;

    return tokens;
}

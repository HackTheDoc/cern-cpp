#include <iostream>
#include <fstream>
#include <sstream>

#include <optional>
#include <vector>

enum TokenType {
    RETURN,
    INTEGER_LITERAL
};

struct Token {
    TokenType type;
    std::optional<std::string> val {};
};

std::vector<Token> tokenize(const std::string& str) {
    std::vector<Token> tokens;
    std::string buf;

    for (unsigned int i = 0; i < str.length(); i++) {
        char c = str[i];

        if (std::isspace(c))
            continue;
        else if (std::isalpha(c)) {
            buf.push_back(c);
            i++;
            while (std::isalnum(str[i])) {
                buf.push_back(str[i]);
                i++;
            }
            i--;

            if (buf == "return") {
                tokens.push_back({.type = TokenType::RETURN});
                buf.clear();
            }
            else {
                std::cerr << "ERROR" << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        else if (std::isdigit(c)) {
            buf.push_back(c);
            i++;
            while (std::isdigit(str[i])) {
                buf.push_back(str[i]);
                i++;
            }
            i--;
            tokens.push_back({.type = TokenType::INTEGER_LITERAL, .val = buf});
        }
        else {
            std::cerr << "ERROR" << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    return tokens;
}

std::string tokens_to_asm(const std::vector<Token>& tokens) {
    std::stringstream output;
    output << "global _start\n";
    output << "_start:\n";

    for (unsigned int i = 0; i < tokens.size(); i++) {
        const Token& token = tokens[i];

        switch (token.type) {
        case TokenType::RETURN:
            if (i+1 < tokens.size() && tokens.at(i+1).type == TokenType::INTEGER_LITERAL)
                output << "  mov rax, 60\n";
                output << "  mov rdi, " << tokens.at(i+1).val.value() << "\n";
                output << "  syscall\n";
            break;
        
        default:
            break;
        }
    }

    return output.str();
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "usage: ds <file.hy>" << std::endl;
        return EXIT_FAILURE;
    }

    std::string contents;
    {
        std::ifstream infile(argv[1]);
        std::stringstream content_stream;
        content_stream << infile.rdbuf();
        contents = content_stream.str();
    }

    std::vector<Token> tokens = tokenize(contents);
    {
        std::ofstream outfile("out.asm");
        outfile << tokens_to_asm(tokens);
    }

    system("nasm -felf64 out.asm");
    system("ld -o out out.o");

    return EXIT_SUCCESS;
}

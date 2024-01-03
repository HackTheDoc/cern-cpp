#include <iostream>
#include <fstream>
#include <sstream>

#include "generation.hpp"

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

    Tokenizer tokenizer(std::move(contents));
    std::vector<Token> tokens = tokenizer.tokenize();

    Parser parser(std::move(tokens));
    std::optional<Node::Prog> prog = parser.parse_prog();

    if (!prog.has_value()) {
        std::cerr << "invalid program" << std::endl;
        exit(EXIT_FAILURE);
    }

    Generator generator(prog.value());
    {
        std::ofstream outfile("out.asm");
        outfile << generator.generate_prog();
    }

    system("nasm -felf64 out.asm");
    system("ld -o out out.o");

    return EXIT_SUCCESS;
}

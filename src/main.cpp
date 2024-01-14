#include <iostream>
#include <fstream>
#include <sstream>

#include "generation.h"

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "usage: cern <file.ce>" << std::endl;
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

    if (!prog.has_value())
    {
        std::cerr << "invalid program" << std::endl;
        exit(EXIT_FAILURE);
    }

    {
        std::ofstream outfile("main.cpp");
        outfile << gen::prog(std::move(prog.value()));
    }

    //system("g++ -std=c++23 main.cpp -o app");
    system("g++ -std=c++23 -Wall -Wextra main.cpp -o app"); // to remove

    return EXIT_SUCCESS;
}

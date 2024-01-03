#pragma once

#include <sstream>

#include "parser.hpp"

class Generator {
private:
    const Node::Return _root;

public:
    Generator(const Node::Return& root) : _root(std::move(root)) {}

    std::string generate() {
        std::stringstream output;
        output << "global _start\n";
        output << "_start:\n";
        output << "  mov rax, 60\n";
        output << "  mov rdi, " << _root.expr.val.val.value() << "\n";
        output << "  syscall\n";

        return output.str();
    }
};

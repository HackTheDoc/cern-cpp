#pragma once

#include <unordered_map>
#include <sstream>

#include "parser.hpp"

class Generator
{
private:
    const Node::Prog _prog;
    std::stringstream _output;
    size_t _stack_size = 0;

    struct Var {
        size_t stack_loc;
    };
    std::unordered_map<std::string, Var> _vars{};

    void push(const std::string& reg)
    {
        _output << "  push " << reg << "\n";
        _stack_size++;
    }

    void pop(const std::string& reg)
    {
        _output << "  pop " << reg << "\n";
        _stack_size--;
    }

public:
    Generator(const Node::Prog& prog) : _prog(std::move(prog)) {}

    void generate_expr(const Node::Expr& expr)
    {
        struct ExprVisitor {
            Generator* gen;

            void operator()(const Node::ExprIntegerLiteral& expr_int_lit) const
            {
                gen->_output << "  mov rax, " << expr_int_lit.int_lit.val.value() << "\n";
                gen->push("rax");
            }

            void operator()(const Node::ExprIdentifier& expr_indent) const 
            {
                if (!gen->_vars.contains(expr_indent.ident.val.value()))
                {
                    std::cerr << "unknown variable '" << expr_indent.ident.val.value() << "'" << std::endl;
                    exit(EXIT_FAILURE);
                }

                const auto& var = gen->_vars[expr_indent.ident.val.value()];
                std::stringstream offset;
                offset << "QWORD [rsp + " << (gen->_stack_size - var.stack_loc) * 8 << "]\n";
                gen->push(offset.str());
            }
        };

        ExprVisitor visitor{.gen = this};
        std::visit(visitor, expr.var);
    }

    void generate_stmt(const Node::Stmt& stmt)
    {
        struct StmtVisitor {
            Generator* gen;

            void operator()(const Node::StmtReturn& stmt_return) const
            {
                gen->generate_expr(stmt_return.expr);
                gen->_output << "  mov rax, 60\n";
                gen->pop("rdi");
                gen->_output << "  syscall\n";
            }

            void operator()(const Node::StmtLet& stmt_let) const
            {
                if (gen->_vars.contains(stmt_let.identifier.val.value()))
                {
                    std::cerr << "'" << stmt_let.identifier.val.value() << "' already used" << std::endl;
                    exit(EXIT_FAILURE);
                }

                gen->generate_expr(stmt_let.expr);
                gen->_vars.insert({stmt_let.identifier.val.value(), Var{gen->_stack_size}});

            }
        };

        StmtVisitor visitor{.gen = this};
        std::visit(visitor, stmt.var);
    }

    std::string generate_prog()
    {
        _output << "global _start\n";
        _output << "_start:\n";

        for (const Node::Stmt& stmt : _prog.stmts)
            generate_stmt(stmt);

        // default exit
        _output << "\n";
        _output << "  mov rax, 60\n";
        _output << "  mov rdi, 0\n";
        _output << "  syscall\n";

        return _output.str();
    }
};

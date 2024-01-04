#pragma once

#include <unordered_map>
#include <sstream>

#include <cassert>

#include "parser.hpp"

class Generator
{
private:
    const Node::Prog _prog;
    std::stringstream _output;
    size_t _stack_size = 0;

    struct Var
    {
        size_t stack_loc;
    };
    std::unordered_map<std::string, Var> _vars{};

    void push(const std::string &reg)
    {
        _output << "  push " << reg << "\n";
        _stack_size++;
    }

    void pop(const std::string &reg)
    {
        _output << "  pop " << reg << "\n";
        _stack_size--;
    }

public:
    Generator(const Node::Prog &prog) : _prog(std::move(prog)) {}

    void generate_term(const Node::Term *term)
    {
        struct TermVisitor
        {
            Generator *gen;

            void operator()(const Node::TermIntegerLiteral *term_int_lit) const
            {
                gen->_output << "  mov rax, " << term_int_lit->int_lit.val.value() << "\n";
                gen->push("rax");
            }

            void operator()(const Node::TermIdentifier *term_ident) const
            {
                if (!gen->_vars.contains(term_ident->ident.val.value()))
                {
                    exit_with("unknown variable '"+term_ident->ident.val.value()+"'");
                }

                const auto &var = gen->_vars[term_ident->ident.val.value()];
                std::stringstream offset;
                offset << "QWORD [rsp + " << (gen->_stack_size - var.stack_loc) * 8 << "]\n";
                gen->push(offset.str());
            }

            void operator()(const Node::TermParen *term_paren) const
            {
                gen->generate_expr(term_paren->expr);
            }
        };

        TermVisitor visitor{.gen = this};
        std::visit(visitor, term->var);
    }

    void generate_bin_exp(const Node::BinExpr *bin_expr)
    {
        struct BinExprVisitor
        {
            Generator *gen;

            void operator()(const Node::BinExprAdd *add) const
            {
                gen->generate_expr(add->lside);
                gen->generate_expr(add->rside);
                gen->pop("rbx");
                gen->pop("rax");
                gen->_output << "  add rax, rbx\n";
                gen->push("rax");
            }

            void operator()(const Node::BinExprSub *sub) const
            {
                gen->generate_expr(sub->lside);
                gen->generate_expr(sub->rside);
                gen->pop("rbx");
                gen->pop("rax");
                gen->_output << "  sub rax, rbx\n";
                gen->push("rax");
            }

            void operator()(const Node::BinExprMulti *multi) const
            {
                gen->generate_expr(multi->lside);
                gen->generate_expr(multi->rside);
                gen->pop("rbx");
                gen->pop("rax");
                gen->_output << "  mul rbx\n";
                gen->push("rax");
            }

            void operator()(const Node::BinExprDiv *div) const
            {
                gen->generate_expr(div->lside);
                gen->generate_expr(div->rside);
                gen->pop("rbx");
                gen->pop("rax");
                gen->_output << "  div rbx\n";
                gen->push("rax");
            }
        };

        BinExprVisitor visitor{.gen = this};
        std::visit(visitor, bin_expr->var);
    }

    void generate_expr(const Node::Expr *expr)
    {
        struct ExprVisitor
        {
            Generator *gen;

            void operator()(const Node::Term *term) const
            {
                gen->generate_term(term);
            }

            void operator()(const Node::BinExpr *expr_bin) const
            {
                gen->generate_bin_exp(expr_bin);
            }
        };

        ExprVisitor visitor{.gen = this};
        std::visit(visitor, expr->var);
    }

    void generate_stmt(const Node::Stmt *stmt)
    {
        struct StmtVisitor
        {
            Generator *gen;

            void operator()(const Node::StmtReturn *stmt_return) const
            {
                gen->generate_expr(stmt_return->expr);
                gen->_output << "  mov rax, 60\n";
                gen->pop("rdi");
                gen->_output << "  syscall\n";
            }

            void operator()(const Node::StmtLet *stmt_let) const
            {
                if (gen->_vars.contains(stmt_let->identifier.val.value()))
                {
                    exit_with("'"+stmt_let->identifier.val.value()+"' already used");
                }

                gen->generate_expr(stmt_let->expr);
                gen->_vars.insert({stmt_let->identifier.val.value(), Var{gen->_stack_size}});
            }
        };

        StmtVisitor visitor{.gen = this};
        std::visit(visitor, stmt->var);
    }

    std::string generate_prog()
    {
        _output << "global _start\n";
        _output << "_start:\n";

        for (const Node::Stmt *stmt : _prog.stmts)
            generate_stmt(stmt);

        // default exit
        _output << "\n";
        _output << "  mov rax, 60\n";
        _output << "  mov rdi, 0\n";
        _output << "  syscall\n";

        return _output.str();
    }
};

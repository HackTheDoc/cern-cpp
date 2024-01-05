#pragma once

#include <sstream>
#include <cassert>
#include <algorithm>

#include "parser.hpp"

class Generator
{
private:
    const Node::Prog _prog;
    std::stringstream _output;
    size_t _stack_size = 0;

    struct Var
    {
        std::string name;
        size_t stack_loc;
    };
    std::vector<Var> _vars{};

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

    std::vector<size_t> _scopes{};

    void begin_scope()
    {
        _scopes.push_back(_vars.size());
    }

    void end_scope()
    {
        const size_t pop_count = _vars.size() - _scopes.back();
        _output << "  add rsp, " << pop_count * 8 << "\n";

        _stack_size -= pop_count;

        for (size_t i = 0; i < pop_count; i++)
            _vars.pop_back();

        _scopes.pop_back();
    }

    size_t lbl_counter = 0;
    std::string create_label()
    {
        std::stringstream ss;
        ss << "label_" << lbl_counter++;
        return ss.str();
    }

    static void exit_with(const std::string &err_msg)
    {
        std::cerr << "[Generation Error] " << err_msg << std::endl;
        exit(EXIT_FAILURE);
    }

public:
    Generator(const Node::Prog &prog) : _prog(std::move(prog)) {}

    void generate_term(const Node::Term *term)
    {
        struct TermVisitor
        {
            Generator &gen;

            void operator()(const Node::TermIntegerLiteral *term_int_lit) const
            {
                gen._output << "  mov rax, " << term_int_lit->int_lit.val.value() << "\n";
                gen.push("rax");
            }

            void operator()(const Node::TermIdentifier *term_ident) const
            {
                const auto it = std::find_if(
                    gen._vars.cbegin(),
                    gen._vars.cend(),
                    [&](const Var &var)
                    {
                        return var.name == term_ident->ident.val.value();
                    });
                if (it == gen._vars.cend())
                {
                    exit_with("unknown variable '" + term_ident->ident.val.value() + "'");
                }

                std::stringstream offset;
                offset << "QWORD [rsp + " << (gen._stack_size - it->stack_loc) * 8 << "]";
                gen.push(offset.str());
            }

            void operator()(const Node::TermParen *term_paren) const
            {
                gen.generate_expr(term_paren->expr);
            }
        };

        TermVisitor visitor{.gen = *this};
        std::visit(visitor, term->var);
    }

    void generate_bin_exp(const Node::BinExpr *bin_expr)
    {
        struct BinExprVisitor
        {
            Generator &gen;

            void operator()(const Node::BinExprAdd *add) const
            {
                gen.generate_expr(add->lside);
                gen.generate_expr(add->rside);
                gen.pop("rbx");
                gen.pop("rax");
                gen._output << "  add rax, rbx\n";
                gen.push("rax");
            }

            void operator()(const Node::BinExprSub *sub) const
            {
                gen.generate_expr(sub->lside);
                gen.generate_expr(sub->rside);
                gen.pop("rbx");
                gen.pop("rax");
                gen._output << "  sub rax, rbx\n";
                gen.push("rax");
            }

            void operator()(const Node::BinExprMulti *multi) const
            {
                gen.generate_expr(multi->lside);
                gen.generate_expr(multi->rside);
                gen.pop("rbx");
                gen.pop("rax");
                gen._output << "  mul rbx\n";
                gen.push("rax");
            }

            void operator()(const Node::BinExprDiv *div) const
            {
                gen.generate_expr(div->lside);
                gen.generate_expr(div->rside);
                gen.pop("rbx");
                gen.pop("rax");
                gen._output << "  div rbx\n";
                gen.push("rax");
            }
        };

        BinExprVisitor visitor{.gen = *this};
        std::visit(visitor, bin_expr->var);
    }

    void generate_expr(const Node::Expr *expr)
    {
        struct ExprVisitor
        {
            Generator &gen;

            void operator()(const Node::Term *term) const
            {
                gen.generate_term(term);
            }

            void operator()(const Node::BinExpr *expr_bin) const
            {
                gen.generate_bin_exp(expr_bin);
            }
        };

        ExprVisitor visitor{.gen = *this};
        std::visit(visitor, expr->var);
    }

    void generate_scope(const Node::Scope *scope)
    {
        begin_scope();

        for (const Node::Stmt *stmt : scope->stmts)
            generate_stmt(stmt);

        end_scope();
    }

    void generate_if_pred(const Node::IfPred *pred, const std::string &end_if_label)
    {
        struct PredVisitor
        {
            Generator &gen;
            const std::string &end_label;

            void operator()(const Node::IfPredElif *elif_pred) const
            {
                gen._output << "  ;; elif\n";

                gen.generate_expr(elif_pred->expr);
                gen.pop("rax");

                const std::string lbl = gen.create_label();

                gen._output << "  test rax, rax\n";
                gen._output << "  jz " << lbl << "\n";

                gen.generate_scope(elif_pred->scope);
                gen._output << "  jmp " << end_label << "\n";

                gen._output << lbl << ":\n";

                if (elif_pred->pred.has_value())
                {
                    gen.generate_if_pred(elif_pred->pred.value(), end_label);
                }
            }

            void operator()(const Node::IfPredElse *else_pred) const
            {
                gen._output << "  ;; else\n";

                gen.generate_scope(else_pred->scope);
            }
        };

        PredVisitor visitor{.gen = *this, .end_label = end_if_label};
        std::visit(visitor, pred->var);
    }

    void generate_stmt(const Node::Stmt *stmt)
    {
        struct StmtVisitor
        {
            Generator &gen;

            void operator()(const Node::StmtReturn *stmt_return) const
            {
                gen._output << "  ;; return\n";

                gen.generate_expr(stmt_return->expr);
                gen._output << "  mov rax, 60\n";
                gen.pop("rdi");
                gen._output << "  syscall\n";

                gen._output << "  ;; /return\n";
            }

            void operator()(const Node::StmtLet *stmt_let) const
            {
                gen._output << "  ;; let\n";

                if (std::find_if(
                        gen._vars.cbegin(),
                        gen._vars.cend(),
                        [&](const Var &var)
                        {
                            return var.name == stmt_let->identifier.val.value();
                        }) != gen._vars.cend())
                {
                    exit_with("identifier '" + stmt_let->identifier.val.value() + "' already used");
                }

                gen.generate_expr(stmt_let->expr);
                gen._vars.push_back({.name = stmt_let->identifier.val.value(), .stack_loc = gen._stack_size});

                gen._output << "  ;; /let\n";
            }

            void operator()(const Node::StmtVarAssign *var_assign) const
            {
                gen._output << "  ;; var assign\n";
                
                const auto it = std::find_if(
                    gen._vars.cbegin(),
                    gen._vars.cend(),
                    [&](const Var &var)
                    {
                        return var.name == var_assign->ident.val.value();
                    });
                if (it == gen._vars.cend())
                {
                    exit_with("unknown identifier '" + var_assign->ident.val.value() + "'");
                }

                gen.generate_expr(var_assign->expr);
                gen.pop("rax");
                gen._output << "  mov [rsp + " << (gen._stack_size - it->stack_loc) * 8 << "], rax\n";

                gen._output << "  ;; /var assign\n";
            }

            void operator()(const Node::Scope *scope) const
            {
                gen._output << " ;; scope\n";

                gen.generate_scope(scope);

                gen._output << "  ;; /scope\n";
            }

            void operator()(const Node::StmtIf *stmt_if) const
            {
                gen._output << "  ;; if\n";
                gen.generate_expr(stmt_if->expr);
                gen.pop("rax");

                const std::string lbl = gen.create_label();

                gen._output << "  test rax, rax\n";
                gen._output << "  jz " << lbl << "\n";
                gen.generate_scope(stmt_if->scope);

                gen._output << lbl << ":\n";

                if (stmt_if->pred.has_value())
                {
                    const std::string end_label = gen.create_label();
                    gen.generate_if_pred(stmt_if->pred.value(), end_label);
                    gen._output << end_label << ":\n";
                }
            
                gen._output << "  ;; /if\n";
            }
        };

        StmtVisitor visitor{.gen = *this};
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

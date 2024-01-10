#pragma once

#include <map>
#include <sstream>
#include <cassert>
#include <algorithm>

#include "parser.hpp"

class Generator
{
private:
    const Node::Prog _prog;

    std::vector<std::string> identifiers;

    std::stringstream _output;
    std::stringstream _main_func;
    std::stringstream current_scope;

    std::string indentation;

    void begin_scope()
    {
        _main_func.copyfmt(current_scope);
        current_scope.clear();

        current_scope << indentation << "{\n";

        indentation += "  ";
    }

    void end_scope()
    {
        indentation.pop_back();
        indentation.pop_back();

        current_scope << indentation << "}\n";

        _main_func << current_scope.str();

        current_scope.clear();
        current_scope.copyfmt(_main_func);
    }

    static void exit_with(const std::string &err_msg)
    {
        std::cerr << "[Generation Error] " << err_msg << std::endl;
        exit(EXIT_FAILURE);
    }

public:
    Generator(const Node::Prog &prog) : _prog(std::move(prog)) {}

    std::string generate_term(const Node::Term *term)
    {
        struct TermVisitor
        {
            Generator &gen;
            std::string result;

            void operator()(const Node::TermIntegerLiteral *term_int_lit)
            {
                result = term_int_lit->int_lit.val.value();
            }

            void operator()(const Node::TermIdentifier *term_ident)
            {
                result = term_ident->ident.val.value();
            }

            void operator()(const Node::TermParen *term_paren)
            {
                result = "(" + gen.generate_expr(term_paren->expr) + ")";
            }
        };

        TermVisitor visitor{.gen = *this};
        std::visit(visitor, term->var);

        return visitor.result;
    }

    std::string generate_bin_exp(const Node::BinExpr *bin_expr)
    {
        struct BinExprVisitor
        {
            Generator &gen;
            std::string result;

            void operator()(const Node::BinExprAdd *add)
            {
                result = gen.generate_expr(add->lside) + " + " + gen.generate_expr(add->rside);
            }

            void operator()(const Node::BinExprSub *sub)
            {
                result = gen.generate_expr(sub->lside) + " - " + gen.generate_expr(sub->rside);
            }

            void operator()(const Node::BinExprMulti *multi)
            {
                result = gen.generate_expr(multi->lside) + " * " + gen.generate_expr(multi->rside);
            }

            void operator()(const Node::BinExprDiv *div)
            {
                result = gen.generate_expr(div->lside) + " / " + gen.generate_expr(div->rside);
            }
        };

        BinExprVisitor visitor{.gen = *this};
        std::visit(visitor, bin_expr->var);

        return visitor.result;
    }

    std::string generate_expr(const Node::Expr *expr)
    {
        struct ExprVisitor
        {
            Generator &gen;
            std::string result;

            void operator()(const Node::Term *term)
            {
                result = gen.generate_term(term);
            }

            void operator()(const Node::BinExpr *expr_bin)
            {
                result = gen.generate_bin_exp(expr_bin);
            }
        };

        ExprVisitor visitor{.gen = *this};
        std::visit(visitor, expr->var);

        return visitor.result;
    }

    void generate_scope(const Node::Scope *scope)
    {
        begin_scope();

        for (const Node::Stmt *stmt : scope->stmts)
            generate_stmt(stmt);

        end_scope();
    }

    void generate_if_pred(const Node::IfPred *pred)
    {
        struct PredVisitor
        {
            Generator &gen;

            void operator()(const Node::IfPredElif *elif_pred) const
            {
                gen.current_scope << gen.indentation;
                gen.current_scope << "else if (";
                gen.current_scope << gen.generate_expr(elif_pred->expr);
                gen.current_scope << ")\n";
                gen.generate_scope(elif_pred->scope);

                if (elif_pred->pred.has_value())
                    gen.generate_if_pred(elif_pred->pred.value());
            }

            void operator()(const Node::IfPredElse *else_pred) const
            {
                gen.current_scope << gen.indentation;
                gen.current_scope << "else\n";
                gen.generate_scope(else_pred->scope);
            }
        };

        PredVisitor visitor{.gen = *this};
        std::visit(visitor, pred->var);
    }

    void generate_stmt(const Node::Stmt *stmt)
    {
        struct StmtVisitor
        {
            Generator &gen;

            void operator()(const Node::StmtReturn *stmt_return) const
            {
                gen.current_scope << gen.indentation;
                gen.current_scope << "return " << gen.generate_expr(stmt_return->expr) << ";\n";
            }

            void operator()(const Node::StmtImplicitVar *stmt_var) const
            {
                if (std::find_if(
                        gen.identifiers.cbegin(),
                        gen.identifiers.cend(),
                        [&](const std::string &ident)
                        {
                            return ident == stmt_var->identifier.val.value();
                        }) != gen.identifiers.cend())
                {
                    exit_with("identifier '" + stmt_var->identifier.val.value() + "' already used");
                }

                gen.identifiers.push_back(stmt_var->identifier.val.value());

                gen.current_scope << gen.indentation;
                gen.current_scope << to_string(stmt_var->expr->type);
                gen.current_scope << " ";
                gen.current_scope << stmt_var->identifier.val.value();
                gen.current_scope << " = ";
                gen.current_scope << gen.generate_expr(stmt_var->expr);
                gen.current_scope << ";\n";
            }

            void operator()(const Node::StmtExplicitVar *stmt_var) const
            {
                if (std::find_if(
                        gen.identifiers.cbegin(),
                        gen.identifiers.cend(),
                        [&](const std::string &ident)
                        {
                            return ident == stmt_var->ident.val.value();
                        }) != gen.identifiers.cend())
                {
                    exit_with("identifier '" + stmt_var->ident.val.value() + "' already used");
                }

                gen.identifiers.push_back(stmt_var->ident.val.value());

                gen.current_scope << gen.indentation;
                gen.current_scope << to_string(stmt_var->type);
                gen.current_scope << " ";
                gen.current_scope << stmt_var->ident.val.value();
                gen.current_scope << ";\n";
            }

            void operator()(const Node::StmtVarAssign *var_assign) const
            {
                const auto it = std::find_if(
                    gen.identifiers.cbegin(),
                    gen.identifiers.cend(),
                    [&](const std::string &ident)
                    {
                        return ident == var_assign->ident.val.value();
                    });
                if (it == gen.identifiers.cend())
                {
                    exit_with("unknown identifier '" + var_assign->ident.val.value() + "'");
                }

                gen.current_scope << gen.indentation;
                gen.current_scope << var_assign->ident.val.value();
                gen.current_scope << " = ";
                gen.current_scope << gen.generate_expr(var_assign->expr);
                gen.current_scope << ";\n";
            }

            void operator()(const Node::Scope *scope) const
            {
                gen.generate_scope(scope);
            }

            void operator()(const Node::StmtIf *stmt_if) const
            {
                gen.current_scope << gen.indentation;
                gen.current_scope << "if (";
                gen.current_scope << gen.generate_expr(stmt_if->expr);
                gen.current_scope << ")\n";
                gen.generate_scope(stmt_if->scope);

                if (stmt_if->pred.has_value())
                    gen.generate_if_pred(stmt_if->pred.value());
            }
        };

        StmtVisitor visitor{.gen = *this};
        std::visit(visitor, stmt->var);
    }

    std::string generate_prog()
    {
        _output << "#include <iostream>\n";

        indentation += "  ";

        for (const Node::Stmt *stmt : _prog.stmts)
            generate_stmt(stmt);

        indentation.pop_back();
        indentation.pop_back();

        _output << "\nint main(int argc, char *argv[]) {\n";

        _output << current_scope.str();

        _output << "  return 0;\n";
        _output << "}\n";

        return _output.str();
    }
};

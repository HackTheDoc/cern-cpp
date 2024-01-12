#include "generation.h"

#include "buildin.hpp"

#include <sstream>
#include <cassert>
#include <algorithm>

namespace gen
{
    namespace {
        std::vector<std::string> identifiers{};

        std::stringstream output;
        std::stringstream main_func;
        std::stringstream current_scope;

        std::string indentation;
    }

    void begin_scope()
    {
        main_func.copyfmt(current_scope);
        current_scope.clear();

        current_scope << indentation << "{\n";

        indentation += "  ";
    }

    void end_scope()
    {
        indentation.pop_back();
        indentation.pop_back();

        current_scope << indentation << "}\n";

        main_func << current_scope.str();

        current_scope.clear();
        current_scope.copyfmt(main_func);
    }

    void exit_with(const std::string &err_msg)
    {
        std::cerr << "[Generation Error] " << err_msg << std::endl;
        exit(EXIT_FAILURE);
    }    

    std::string prog(const Node::Prog p)
    {
        output << "#include <iostream>\n";

        indentation += "  ";

        for (const Node::Stmt *s : p.stmts)
            stmt(s);

        indentation.pop_back();
        indentation.pop_back();

        output << "\nint main() {\n";

        output << current_scope.str();

        output << "  return 0;\n";
        output << "}\n";

        return output.str();
    }

    void stmt(const Node::Stmt *s)
    {
        struct StmtVisitor
        {
            void operator()(const Node::StmtReturn *stmt_return) const
            {
                current_scope << indentation;
                current_scope << "return " << expr(stmt_return->expr) << ";\n";
            }

            void operator()(const Node::StmtImplicitVar *stmt_var) const
            {
                if (std::find_if(
                        identifiers.cbegin(),
                        identifiers.cend(),
                        [&](const std::string &ident)
                        {
                            return ident == stmt_var->identifier.val.value();
                        }) != identifiers.cend())
                {
                    exit_with("identifier '" + stmt_var->identifier.val.value() + "' already used");
                }

                identifiers.push_back(stmt_var->identifier.val.value());

                current_scope << indentation;
                current_scope << to_string(stmt_var->expr->type);
                current_scope << " ";
                current_scope << stmt_var->identifier.val.value();
                current_scope << " = ";
                current_scope << expr(stmt_var->expr);
                current_scope << ";\n";
            }

            void operator()(const Node::StmtExplicitVar *stmt_var) const
            {
                if (std::find_if(
                        identifiers.cbegin(),
                        identifiers.cend(),
                        [&](const std::string &ident)
                        {
                            return ident == stmt_var->ident.val.value();
                        }) != identifiers.cend())
                {
                    exit_with("identifier '" + stmt_var->ident.val.value() + "' already used");
                }

                identifiers.push_back(stmt_var->ident.val.value());

                current_scope << indentation;
                current_scope << to_string(stmt_var->type);
                current_scope << " ";
                current_scope << stmt_var->ident.val.value();
                current_scope << ";\n";
            }

            void operator()(const Node::StmtVarAssign *var_assign) const
            {
                if (std::find_if(
                    identifiers.cbegin(),
                    identifiers.cend(),
                    [&](const std::string &ident)
                    {
                        return ident == var_assign->ident.val.value();
                    }) == identifiers.cend())
                {
                    exit_with("unknown identifier '" + var_assign->ident.val.value() + "'");
                }

                current_scope << indentation;
                current_scope << var_assign->ident.val.value();
                current_scope << " = ";
                current_scope << expr(var_assign->expr);
                current_scope << ";\n";
            }

            void operator()(const Node::StmtFuncCall *fcall) const
            {
                if (const auto f = call_func(fcall->ident.val.value(), fcall->args))
                {
                    current_scope << indentation;
                    current_scope << f.value();
                    return;
                }

                if (std::find_if(
                    identifiers.cbegin(),
                    identifiers.cend(),
                    [&](const std::string &ident)
                    {
                        return ident == fcall->ident.val.value();
                    }) == identifiers.cend())
                {
                    exit_with("unknown identifier '" + fcall->ident.val.value() + "'");
                }

                current_scope << indentation;
                current_scope << fcall->ident.val.value();
                current_scope << " (";

                if (fcall->args.has_value())
                {
                    current_scope << expr(fcall->args.value()->expr);

                    auto narg = fcall->args.value()->next_arg;
                    while (narg.has_value())
                    {
                        current_scope << ", ";
                        current_scope << expr(narg.value()->expr);

                        narg = narg.value()->next_arg;
                    }
                }

                current_scope << ");\n";
            }

            void operator()(const Node::Scope *s) const
            {
                scope(s);
            }

            void operator()(const Node::StmtIf *stmt_if) const
            {
                current_scope << indentation;
                current_scope << "if (";
                current_scope << expr(stmt_if->expr);
                current_scope << ")\n";
                scope(stmt_if->scope);

                if (stmt_if->pred.has_value())
                    if_pred(stmt_if->pred.value());
            }
        };

        StmtVisitor visitor;
        std::visit(visitor, s->var);
    }

    void scope(const Node::Scope *sc)
    {
        begin_scope();

        for (const Node::Stmt *s : sc->stmts)
            stmt(s);

        end_scope();
    }

    void if_pred(const Node::IfPred *pred)
    {
        struct PredVisitor
        {
            void operator()(const Node::IfPredElif *elif_pred) const
            {
                current_scope << indentation;
                current_scope << "else if (";
                current_scope << expr(elif_pred->expr);
                current_scope << ")\n";
                scope(elif_pred->scope);

                if (elif_pred->pred.has_value())
                    if_pred(elif_pred->pred.value());
            }

            void operator()(const Node::IfPredElse *else_pred) const
            {
                current_scope << indentation;
                current_scope << "else\n";
                scope(else_pred->scope);
            }
        };

        PredVisitor visitor;
        std::visit(visitor, pred->var);
    }

    std::string expr(const Node::Expr *e)
    {
        struct ExprVisitor
        {
            std::string result;

            void operator()(const Node::Term *t)
            {
                result = term(t);
            }

            void operator()(const Node::BinExpr *bin_e)
            {
                result = bin_expr(bin_e);
            }
        };

        ExprVisitor visitor;
        std::visit(visitor, e->var);

        return visitor.result;
    }

    std::string bin_expr(const Node::BinExpr *bin)
    {
        struct BinExprVisitor
        {
            std::string result;

            void operator()(const Node::BinExprAdd *add)
            {
                result = expr(add->lside) + " + " + expr(add->rside);
            }

            void operator()(const Node::BinExprSub *sub)
            {
                result = expr(sub->lside) + " - " + expr(sub->rside);
            }

            void operator()(const Node::BinExprMulti *multi)
            {
                result = expr(multi->lside) + " * " + expr(multi->rside);
            }

            void operator()(const Node::BinExprDiv *div)
            {
                result = expr(div->lside) + " / " + expr(div->rside);
            }
        };

        BinExprVisitor visitor;
        std::visit(visitor, bin->var);

        return visitor.result;
    }

    std::string term(const Node::Term *t)
    {
        struct TermVisitor
        {
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
                result = "(" + expr(term_paren->expr) + ")";
            }
        };

        TermVisitor visitor;
        std::visit(visitor, t->var);

        return visitor.result;
    }
}

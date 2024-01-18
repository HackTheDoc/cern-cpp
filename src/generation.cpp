#include "generation.h"

#include "buildin.h"

#include <sstream>
#include <cassert>
#include <algorithm>
#include <stack>

namespace gen {
    namespace {
        std::stringstream output;

        std::stringstream current_scope;
        std::stack<std::stringstream> scope_stack;

        std::string indentation;
    }

    void begin_scope() {
        scope_stack.emplace(current_scope.str());

        current_scope.clear();

        current_scope << indentation << "{\n";

        indentation += "  ";
    }

    void end_scope() {
        indentation.pop_back();
        indentation.pop_back();

        current_scope << indentation << "}\n";

        scope_stack.top() << current_scope.str();

        current_scope.clear();

        current_scope.copyfmt(scope_stack.top());

        scope_stack.pop();
    }

    void exit_with(const std::string& err_msg) {
        std::cerr << "[Generation Error] " << err_msg << std::endl;
        exit(EXIT_FAILURE);
    }

    std::string prog(const Node::Prog p) {
        output << "#include <iostream>\n\n";

        for (const Node::ProgStmt* s : p.stmts)
            prog_stmt(s);

        output << current_scope.str();

        return output.str();
    }

    void prog_stmt(const Node::ProgStmt* s) {
        struct ProgStmtVisitor {
            void operator()(const Node::StmtImplicitVar* stmt_var) const {
                current_scope << indentation;
                current_scope << to_string(stmt_var->expr->type);
                current_scope << " ";
                current_scope << stmt_var->identifier.val.value();
                current_scope << " = ";
                current_scope << expr(stmt_var->expr);
                current_scope << ";\n";
            }

            void operator()(const Node::StmtExplicitVar* stmt_var) const {
                current_scope << indentation;
                current_scope << to_string(stmt_var->type);
                current_scope << " ";
                current_scope << stmt_var->ident.val.value();
                current_scope << ";\n";
            }

            void operator()(const Node::FuncDeclaration* func) const {
                current_scope << "\n";
                current_scope << indentation;
                current_scope << to_string(func->type);
                current_scope << " ";
                current_scope << func->ident.val.value();
                current_scope << "()\n";
                scope(func->scope);
            }
        };

        ProgStmtVisitor visitor;
        std::visit(visitor, s->var);
    }

    void scope(const Node::Scope* sc) {
        begin_scope();

        for (const Node::ScopeStmt* s : sc->stmts)
            scope_stmt(s);

        end_scope();
    }

    void scope_stmt(const Node::ScopeStmt* s) {
        struct ScopeStmtVisitor {
            void operator()(const Node::StmtReturn* stmt_return) const {
                current_scope << indentation;
                current_scope << "return " << expr(stmt_return->expr) << ";\n";
            }

            void operator()(const Node::StmtImplicitVar* stmt_var) const {
                current_scope << indentation;
                current_scope << to_string(stmt_var->expr->type);
                current_scope << " ";
                current_scope << stmt_var->identifier.val.value();
                current_scope << " = ";
                current_scope << expr(stmt_var->expr);
                current_scope << ";\n";
            }

            void operator()(const Node::StmtExplicitVar* stmt_var) const {
                current_scope << indentation;
                current_scope << to_string(stmt_var->type);
                current_scope << " ";
                current_scope << stmt_var->ident.val.value();
                current_scope << ";\n";
            }

            void operator()(const Node::StmtVarAssign* var_assign) const {
                current_scope << indentation;
                current_scope << var_assign->ident.val.value();
                current_scope << " = ";
                current_scope << expr(var_assign->expr);
                current_scope << ";\n";
            }

            void operator()(const Node::FuncCall* fcall) const {
                if (const auto f = call_func(fcall->ident.val.value(), fcall->args)) {
                    current_scope << indentation;
                    current_scope << f.value();
                    return;
                }

                current_scope << indentation;
                current_scope << fcall->ident.val.value();
                current_scope << " (";

                if (!fcall->args.empty()) {
                    current_scope << expr(fcall->args[0]);

                    for (size_t i = 1; i < fcall->args.size(); i++) {
                        current_scope << ", " << expr(fcall->args[i]);
                    }
                }

                current_scope << ");\n";
            }

            void operator()(const Node::VarIncr* i) const {
                current_scope << indentation;
                current_scope <<  i->ident->ident.val.value();
                current_scope << "++;\n";
            }

            void operator()(const Node::VarDecr* d) const {
                current_scope << indentation;
                current_scope <<  d->ident->ident.val.value();
                current_scope << "--;\n";
            }

            void operator()(const Node::Scope* s) const {
                scope(s);
            }

            void operator()(const Node::StmtWhile* w) const {
                current_scope << indentation;
                current_scope << "while (";
                current_scope << expr(w->expr);
                current_scope << ")\n";
                scope(w->scope);
            }

            void operator()(const Node::StmtIf* stmt_if) const {
                current_scope << indentation;
                current_scope << "if (";
                current_scope << expr(stmt_if->expr);
                current_scope << ")\n";
                scope(stmt_if->scope);

                if (stmt_if->pred.has_value())
                    if_pred(stmt_if->pred.value());
            }
        };

        ScopeStmtVisitor visitor;
        std::visit(visitor, s->var);
    }

    void if_pred(const Node::IfPred* pred) {
        struct PredVisitor {
            void operator()(const Node::IfPredElif* elif_pred) const {
                current_scope << indentation;
                current_scope << "else if (";
                current_scope << expr(elif_pred->expr);
                current_scope << ")\n";
                scope(elif_pred->scope);

                if (elif_pred->pred.has_value())
                    if_pred(elif_pred->pred.value());
            }

            void operator()(const Node::IfPredElse* else_pred) const {
                current_scope << indentation;
                current_scope << "else\n";
                scope(else_pred->scope);
            }
        };

        PredVisitor visitor;
        std::visit(visitor, pred->var);
    }

    std::string expr(const Node::Expr* e) {
        struct ExprVisitor {
            std::string result;

            void operator()(const Node::Term* t) {
                result = term(t);
            }

            void operator()(const Node::BinExpr* bin_e) {
                result = bin_expr(bin_e);
            }
        
            void operator()(const Node::ExprNot* e) {
                result = "!" + expr(e->expr);
            }

            void operator()(const Node::VarIncr* i) {
                result = i->ident->ident.val.value() + "++";
            }

            void operator()(const Node::VarDecr* d) {
                result = d->ident->ident.val.value() + "--";
            }
        };

        ExprVisitor visitor;
        std::visit(visitor, e->var);

        return visitor.result;
    }

    std::string bin_expr(const Node::BinExpr* bin) {
        struct BinExprVisitor {
            std::string result;

            void operator()(const Node::BinExprAdd* add) {
                result = expr(add->lside) + " + " + expr(add->rside);
            }

            void operator()(const Node::BinExprSub* sub) {
                result = expr(sub->lside) + " - " + expr(sub->rside);
            }

            void operator()(const Node::BinExprMulti* multi) {
                result = expr(multi->lside) + " * " + expr(multi->rside);
            }

            void operator()(const Node::BinExprDiv* div) {
                result = expr(div->lside) + " / " + expr(div->rside);
            }

            void operator()(const Node::BinExprAnd* e) {
                result = expr(e->lside) + " && " + expr(e->rside);
            }

            void operator()(const Node::BinExprOr* e) {
                result = expr(e->lside) + " || " + expr(e->rside);
            }

            void operator()(const Node::BinExprIsEqual* e) {
                result = expr(e->lside) + " == " + expr(e->rside);
            }

            void operator()(const Node::BinExprIsNotEqual* e) {
                result = expr(e->lside) + " != " + expr(e->rside);
            }

            void operator()(const Node::BinExprGreaterOrEqual* e) {
                result = expr(e->lside) + " >= " + expr(e->rside);
            }

            void operator()(const Node::BinExprGreater* e) {
                result = expr(e->lside) + " > " + expr(e->rside);
            }

            void operator()(const Node::BinExprLowerOrEqual* e) {
                result = expr(e->lside) + " <= " + expr(e->rside);
            }

            void operator()(const Node::BinExprLower* e) {
                result = expr(e->lside) + " < " + expr(e->rside);
            }
        };

        BinExprVisitor visitor;
        std::visit(visitor, bin->var);

        return visitor.result;
    }

    std::string term(const Node::Term* t) {
        struct TermVisitor {
            std::string result;

            void operator()(const Node::TermBooleanLiteral* term_bool_lit) {
                result = term_bool_lit->bool_lit.val.value();
            }

            void operator()(const Node::TermIntegerLiteral* term_int_lit) {
                result = term_int_lit->int_lit.val.value();
            }

            void operator()(const Node::TermCharLiteral* term_char_lit) {
                result = "'" + term_char_lit->char_lit.val.value() + "'";
            }

            void operator()(const Node::TermIdentifier* term_ident) {
                result = term_ident->ident.val.value();
            }

            void operator()(const Node::FuncCall* fcall) {
                if (const auto f = call_func(fcall->ident.val.value(), fcall->args)) {
                    result = f.value();
                    return;
                }

                result = " ";
                result += fcall->ident.val.value();
                result += "(";

                if (!fcall->args.empty()) {
                    result += expr(fcall->args[0]);

                    for (size_t i = 1; i < fcall->args.size(); i++) {
                        result += ", " + expr(fcall->args[i]);
                    }
                }

                result += ")";
            }

            void operator()(const Node::TermParen* term_paren) {
                result = "(" + expr(term_paren->expr) + ")";
            }
        };

        TermVisitor visitor;
        std::visit(visitor, t->var);

        return visitor.result;
    }
}

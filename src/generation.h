#pragma once

#include "parser.h"

namespace gen {
    void begin_scope();

    void end_scope();

    void exit_with(const std::string &err_msg);

    std::string prog(const Node::Prog p);

    void prog_stmt(const Node::ProgStmt *s);

    void scope(const Node::Scope *sc);

    void scope_stmt(const Node::ScopeStmt *s);

    void if_pred(const Node::IfPred *pred);

    std::string expr(const Node::Expr *e);

    std::string bin_expr(const Node::BinExpr *bin);

    std::string term(const Node::Term *t);
}

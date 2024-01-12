#pragma once

#include "parser.h"

namespace gen 
{
    void begin_scope();
    
    void end_scope();

    void exit_with(const std::string& err_msg);

    std::string prog(const Node::Prog p);

    void stmt(const Node::Stmt* s);

    void scope(const Node::Scope* sc);

    void if_pred(const Node::IfPred* pred);

    std::string expr(const Node::Expr* e);

    std::string bin_expr(const Node::BinExpr* bin);

    std::string term(const Node::Term* t);
}

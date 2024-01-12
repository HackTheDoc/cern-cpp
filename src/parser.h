#pragma once

#include <cassert>
#include <variant>

#include "tokenizer.h"
#include "arena.hpp"

enum VarType
{
    NONE,
    INT
};

std::string to_string(VarType t);
VarType to_variable_type(TokenType t);

namespace Node
{
    struct Expr;

    struct TermIntegerLiteral
    {
        Token int_lit;
    };

    struct TermIdentifier
    {
        Token ident;
    };

    struct TermParen
    {
        Expr *expr;
    };

    struct Term
    {
        std::variant<TermIntegerLiteral *, TermIdentifier *, TermParen *> var;
        VarType type{VarType::NONE};
    };

    struct BinExprAdd
    {
        Expr *lside;
        Expr *rside;
    };

    struct BinExprSub
    {
        Expr *lside;
        Expr *rside;
    };

    struct BinExprMulti
    {
        Expr *lside;
        Expr *rside;
    };

    struct BinExprDiv
    {
        Expr *lside;
        Expr *rside;
    };

    struct BinExpr
    {
        std::variant<BinExprAdd *, BinExprSub *, BinExprMulti *, BinExprDiv *> var;
    };

    struct Expr
    {
        std::variant<Term *, BinExpr *> var;
        VarType type{VarType::NONE};
    };

    struct Stmt;

    struct Scope
    {
        std::vector<Stmt *> stmts;
    };

    struct StmtReturn
    {
        Expr *expr;
    };

    // var ident = value
    struct StmtImplicitVar
    {
        Token identifier;
        Expr *expr;
    };

    // var ident : type
    struct StmtExplicitVar
    {
        Token ident;
        VarType type;
    };

    struct StmtVarAssign
    {
        Token ident;
        Expr *expr;
    };

    struct ArgList
    {
        Expr* expr;
        std::optional<ArgList*> next_arg;
    };

    struct StmtFuncCall
    {
        Token ident;
        std::optional<ArgList*> args;
    };

    struct IfPred;

    struct IfPredElif
    {
        Expr *expr;
        Scope *scope;
        std::optional<IfPred *> pred;
    };

    struct IfPredElse
    {
        Scope *scope;
    };

    struct IfPred
    {
        std::variant<IfPredElif *, IfPredElse *> var;
    };

    struct StmtIf
    {
        Expr *expr;
        Scope *scope;
        std::optional<IfPred *> pred;
    };

    struct Stmt
    {
        std::variant<
            StmtReturn *,
            StmtImplicitVar *, 
            StmtExplicitVar*, 
            StmtVarAssign *,
            StmtFuncCall*, 
            Scope *, 
            StmtIf *
        > var;
    };

    struct Prog
    {
        std::vector<Stmt *> stmts;
    };
}

class Parser
{
private:
    const std::vector<Token> _tokens;

    size_t _index = 0;

    ArenaAllocator _allocator;

    std::optional<Token> peek(const int offset = 0) const;

    bool peek_type(TokenType type, int offset = 0) const;

    Token consume();

    Token try_consume_err(TokenType type);

    std::optional<Token> try_consume(TokenType type);

    void exit_with(const std::string &err_msg, std::string template_msg = "missing");

public:
    Parser(std::vector<Token> tokens);

    std::optional<Node::Prog> parse_prog();

    std::optional<Node::Stmt *> parse_stmt();

    std::optional<Node::Scope *> parse_scope();

    std::optional<Node::ArgList*> parse_args();

    std::optional<Node::IfPred *> parse_if_pred();

    std::optional<Node::Expr *> parse_expr(int min_prec = 0);

    std::optional<Node::Term *> parse_term();

    std::optional<VarType> parse_type();
};
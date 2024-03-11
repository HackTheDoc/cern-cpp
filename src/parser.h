#pragma once

#include <unordered_map>
#include <cassert>
#include <variant>

#include "tokenizer.h"
#include "arena.hpp"

enum VarType {
    VOID,
    BOOL,
    INT,
    CHAR,
    STRING
};

std::string to_string(VarType t);
VarType to_variable_type(TokenType t);

namespace Node {
    struct Expr;

    struct Scope;

    struct FuncCall {
        Token ident;
        std::vector<Expr*> args;
        VarType type{ VarType::VOID };
    };

    struct TermBooleanLiteral {
        Token bool_lit;
    };

    struct TermIntegerLiteral {
        Token int_lit;
    };

    struct TermCharLiteral {
        Token char_lit;
    };

    struct TermStringLiteral {
        Token string_lit;
    };

    struct TermIdentifier {
        Token ident;
        VarType type{VarType::VOID};
    };

    struct TermParen {
        Expr* expr;
    };

    struct Term {
        std::variant<
            TermBooleanLiteral*,
            TermIntegerLiteral*,
            TermCharLiteral*,
            TermStringLiteral*,
            TermIdentifier*,
            FuncCall*,
            TermParen*>
            var;
        VarType type{ VarType::VOID };
    };

    struct BinExprAdd {
        Expr* lside;
        Expr* rside;
    };

    struct BinExprSub {
        Expr* lside;
        Expr* rside;
    };

    struct BinExprMulti {
        Expr* lside;
        Expr* rside;
    };

    struct BinExprDiv {
        Expr* lside;
        Expr* rside;
    };

    struct BinExprAnd {
        Expr* lside;
        Expr* rside;
    };

    struct BinExprOr {
        Expr* lside;
        Expr* rside;
    };

    struct BinExprIsEqual {
        Expr* lside;
        Expr* rside;
    };

    struct BinExprIsNotEqual {
        Expr* lside;
        Expr* rside;
    };

    struct BinExprGreaterOrEqual {
        Expr* lside;
        Expr* rside;
    };

    struct BinExprGreater {
        Expr* lside;
        Expr* rside;
    };

    struct BinExprLowerOrEqual {
        Expr* lside;
        Expr* rside;
    };

    struct BinExprLower {
        Expr* lside;
        Expr* rside;
    };

    struct BinExpr {
        std::variant<
            BinExprAdd*,
            BinExprSub*,
            BinExprMulti*,
            BinExprDiv*,

            BinExprAnd*,
            BinExprOr*,
            BinExprIsEqual*,
            BinExprIsNotEqual*,
            BinExprGreaterOrEqual*,
            BinExprGreater*,
            BinExprLowerOrEqual*,
            BinExprLower*
        > var;
    };

    struct ExprNot {
        Expr* expr;
    };

    struct VarIncr {
        TermIdentifier* ident;
    };

    struct VarDecr {
        TermIdentifier* ident;
    };

    struct Expr {
        std::variant<
            Term*,
            BinExpr*,
            ExprNot*,
            VarIncr*,
            VarDecr*
        > var;
        VarType type{ VarType::VOID };
    };

    struct ScopeStmt;

    // var ident = value
    struct StmtImplicitVar {
        Token identifier;
        Expr* expr;
    };

    // var ident : type
    struct StmtExplicitVar {
        Token ident;
        VarType type;
    };

    // func indent() { ? }
    struct FuncDeclaration {
        Token ident;
        Scope* scope;
        VarType type{ VarType::VOID };
    };

    struct StmtVarAssign {
        Token ident;
        Expr* expr;
    };

    struct StmtReturn {
        Expr* expr;
    };

    struct StmtWhile {
        Expr* expr;
        Scope* scope;
    };

    struct IfPred;

    struct IfPredElif {
        Expr* expr;
        Scope* scope;
        std::optional<IfPred*> pred;
    };

    struct IfPredElse {
        Scope* scope;
    };

    struct IfPred {
        std::variant<IfPredElif*, IfPredElse*> var;
    };

    struct StmtIf {
        Expr* expr;
        Scope* scope;
        std::optional<IfPred*> pred;
    };

    struct ScopeStmt {
        std::variant<
            Scope*,
            StmtImplicitVar*,
            StmtExplicitVar*,
            StmtVarAssign*,
            FuncCall*,
            VarIncr*,
            VarDecr*,
            StmtReturn*,
            StmtWhile*,
            StmtIf*
        > var;
        std::optional<VarType> type{};
    };

    struct Scope {
        std::vector<ScopeStmt*> stmts;
        VarType type{ VarType::VOID };
    };

    struct ProgStmt {
        std::variant<
            FuncDeclaration*,
            StmtImplicitVar*,
            StmtExplicitVar*
        > var;
    };

    struct Prog {
        std::vector<ProgStmt*> stmts;
    };
}

class Parser {
private:
    // contains every token in order
    const std::vector<Token> tokens;

    // current token index
    size_t index = 0;

    ArenaAllocator allocator;

    // map the buildin functions and their return type
    static const std::unordered_map<std::string, VarType> buildin_func_type;

    // check if an identifier is a buildin function
    static bool is_buildin_func(const std::string& func);

    // map the identifiers (vars and funcs) with their return type
    static std::unordered_map<std::string, VarType> identifiers;

    // check if an identifier exist or not
    static bool is_var(const std::string& var);

    static std::optional<VarType> get_return_type(VarType t1, TokenType op, VarType t2);

    // parse the type associated with an identifier
    std::optional<VarType> var_type(const std::string& ident);

    // peek the current token (use the offset to check forward or backward)
    std::optional<Token> peek(const int offset = 0) const;

    // check the type of the current token (return false if there is no token left)
    bool peek_type(TokenType type, int offset = 0) const;

    // consume the current token and return it; move to the next token
    Token consume();

    // consume if the token have the given type; else exit with an error
    Token try_consume_err(TokenType type);

    // try to consume a token of a specific type
    std::optional<Token> try_consume(TokenType type);

    /// @brief exit with an error message
    /// @param err_msg content of the error message
    /// @param template_msg balise of it (ex: missing, expected, ...)
    void exit_with(const std::string& err_msg, std::string template_msg = "missing");

public:
    Parser(std::vector<Token> tokens);

    std::optional<Node::Prog> parse_prog();

    std::optional<Node::ProgStmt*> parse_prog_stmt();

    std::optional<Node::Scope*> parse_scope();

    std::optional<Node::ScopeStmt*> parse_scope_stmt();

    std::vector<Node::Expr*> parse_args();

    std::optional<Node::IfPred*> parse_if_pred();

    std::optional<Node::Expr*> parse_expr(int min_prec = 0);

    std::optional<Node::Term*> parse_term();

    std::optional<Node::TermIdentifier*> parse_identifier();

    std::optional<VarType> parse_type();
};

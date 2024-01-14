$$

\begin{aligned}

    [\text{Prog}] &\to [\text{ProgStmt}]^* \\

    [\text{ProgStmt}] &\to
    \begin{cases}
        [\text{FuncDeclaration}] \\
        [\text{VarDeclaration}] \\
    \end{cases} \\

    [\text{ScopeStmt}] &\to
    \begin{cases}
        [\text{Scope}] \\
        [\text{VarDeclaration}] \\
        \text{identifier} = [\text{Expr}] \\
        [\text{FunctionCall}] \\
        \text{return [Expr]} \\
        if\space([\text{Expr}])\space[\text{Scope}]\space[\text{IfPred} \\
    \end{cases} \\

    [\text{Scope}] &\to \{[\text{ScopeStmt}]^*\} \\

    [\text{VarDeclaration}] &\to
    \begin{cases}
        \text{var identifier} = [\text{Expr}] \\
        \text{var identifier} : [\text{Type}] \\
        \text{var identifier} : [\text{Type}] = [\text{Expr}] \\
    \end{cases} \\

    [\text{FuncDeclaration}] &\to
    \begin{cases}
        \text{func identifier}\space()\space[\text{Scope}] \\
        \text{func identifier}\space()\text{ : }[\text{Type}]\space[\text{Scope}] \\
    \end{cases} \\

    [\text{IfPred}] &\to
    \begin{cases}
        elif\space({\text{Expr}})\space[\text{Scope}]\space[\text{IfPred}] \\
        else\space[\text{Scope}] \\
        \varepsilon
    \end{cases} \\

    [\text{Args}] &\to [\text{Expr}]^* \\

    [\text{FunctionCall}] &\to \text{identifier}\space([\text{Args}]) \\

    [\text{Expr}] &\to 
    \begin{cases}
        \text{Term} \\
        \text{BinExpr}
    \end{cases} \\

    [\text{BinaryExpr}] &\to
    \begin{cases}
        [\text{Expr}] * [\text{Expr}] & {prec} = 1\\
        [\text{Expr}] / [\text{Expr}] & {prec} = 1\\
        [\text{Expr}] + [\text{Expr}] & {prec} = 0\\
        [\text{Expr}] - [\text{Expr}] & {prec} = 0\\
    \end{cases} \\

    [\text{Term}] &\to 
    \begin{cases}
        \text{identifier} \\
        [\text{FunctionCall}] \\
        \text{integer\_literal} \\
        \text{char\_literal} \\
        ([\text{Expr}])
    \end{cases} \\

    [\text{Type}] &\to
    \begin{cases}
        int \\
        char \\
    \end{cases} \\

\end{aligned}

$$
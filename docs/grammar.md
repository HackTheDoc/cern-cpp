$$

\begin{aligned}

    [\text{Prog}] &\to [\text{Stmt}]^* \\

    [\text{Stmt}] &\to
    \begin{cases}
        [\text{Scope}] \\
        [\text{VarDeclaration}] \\
        \text{identifier} = [\text{Expr}] \\
        \text{return [Expr]} \\
        if\space([\text{Expr}])\space[\text{Scope}]\space[\text{IfPred} 
    \end{cases} \\

    [\text{Scope}] &\to \{[\text{Stmt}]^*\} \\

    [\text{VarDeclaration}] &\to
    \begin{cases}
        \text{var identifier} = [\text{Expr}] \\
        \text{var identifier} : [\text{Type}] \\
    \end{cases} \\

    [\text{IfPred}] &\to
    \begin{cases}
        elif\space({\text{Expr}})\space[\text{Scope}]\space[\text{IfPred}] \\
        else\space[\text{Scope}] \\
        \epsilon
    \end{cases} \\

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
        \text{integer\_literal} \\
        ([\text{Expr}])
    \end{cases} \\

    [\text{Type}] &\to
    \begin{cases}
        int \\
    \end{cases} \\

\end{aligned}

$$
$$

\begin{aligned}

    [\text{Prog}] &\to [\text{Stmt}]^* \\

    [\text{Stmt}] &\to
    \begin{cases}
        [\text{Scope}] \\
        \text{var identifier} = [\text{Expr}] \\
        \text{identifier} = [\text{Expr}] \\
        \text{return [Expr]} \\
        if\space([\text{Expr}])\space[\text{Scope}]\space[\text{IfPred} 
    \end{cases} \\

    [\text{Scope}] &\to \{[\text{Stmt}]^*\} \\

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
        \text{integer\_literal} \\
        \text{identifier} \\
        ([\text{Expr}])
    \end{cases} \\

\end{aligned}

$$
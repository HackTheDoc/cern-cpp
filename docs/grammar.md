$$

\begin{align}

    [\text{Prog}] &\to [\text{Scope}] \\

    [\text{Scope}] &\to \{[\text{Stmt}]^*\} \\

    [\text{Stmt}] &\to 
    \begin{cases}
        [\text{Scope}] \\
        \text{return [Expr]} \\
        \text{let identifier} = [\text{Expr}] \\
        if\space([\text{Expr}])\space[\text{Scope}]\space[\text{IfPred}] \\

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
        \text{integer\_literal} \\
        \text{identifier} \\
        ([\text{Expr}])
    \end{cases} \\

\end{align}

$$
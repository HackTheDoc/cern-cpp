$$

\begin{align}

    [\text{Prog}] &\to [\text{Stmt}]^* \\

    [\text{Stmt}] &\to 
    \begin{cases}
        \text{return [Expr]} \\
        \text{let identifier} = [\text{Expr}]
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
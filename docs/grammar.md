$$

\begin{aligned}

    [\text{Prog}] &\to [\text{ProgStmt}]^* \\

    [\text{ProgStmt}] &\to
    \begin{cases}
        [\text{FuncDeclaration}] \\
        [\text{VarDeclaration}] \\
    \end{cases} \\
    
    [\text{Scope}] &\to \{[\text{ScopeStmt}]^*\} \\

    [\text{ScopeStmt}] &\to
    \begin{cases}
        [\text{VarDeclaration}] \\
        \text{identifier} = [\text{Expr}] \\
        [\text{FunctionCall}] \\
        [\text{Scope}] \\
        if\space([\text{Expr}])\space[\text{Scope}]\space[\text{IfPred}]\\
        while\space([\text{Expr}])\space[\text{Scope}] \\
        \text{return [Expr]} \\
    \end{cases} \\

    [\text{FuncDeclaration}] &\to
    \begin{cases}
        \text{func identifier}\space()\space[\text{Scope}] \\
        \text{func identifier}\space()\text{ : }[\text{Type}]\space[\text{Scope}] \\
    \end{cases} \\

    [\text{VarDeclaration}] &\to
    \begin{cases}
        \text{var identifier} = [\text{Expr}] \\
        \text{var identifier} : [\text{Type}] = [\text{Expr}] \\
        \text{var identifier} : [\text{Type}] \\
    \end{cases} \\

    [\text{Args}] &\to [\text{Expr}]^* \\

    [\text{FunctionCall}] &\to \text{identifier}\space([\text{Args}]) \\

    [\text{IfPred}] &\to
    \begin{cases}
        elif\space({\text{Expr}})\space[\text{Scope}]\space[\text{IfPred}] \\
        else\space[\text{Scope}] \\
        \varepsilon
    \end{cases} \\

    [\text{Expr}] &\to 
    \begin{cases}
        \text{Term} \\
        \text{BinExpr} \\
        ! \space [\text{Expr}] \\
        [\text{Expr}]++ \\
        [\text{Expr}]-- \\
    \end{cases} \\

    [\text{BinaryExpr}] &\to
    \begin{cases}
        [\text{Expr}] \space * \space [\text{Expr}] & {prec} = 1\\
        [\text{Expr}] \space / \space [\text{Expr}] & {prec} = 1\\
        [\text{Expr}] \space + \space [\text{Expr}] & {prec} = 0\\
        [\text{Expr}] \space - \space [\text{Expr}] & {prec} = 0\\
        \\
        [\text{Expr}]\space \&\& \space[\text{Expr}] & {prec} = 1\\
        [\text{Expr}]\space || \space [\text{Expr}] & {prec} = 1\\
        [\text{Expr}] == [\text{Expr}] & {prec} = 0\\
        [\text{Expr}]\space != [\text{Expr}] & {prec} = 0\\
        [\text{Expr}] > [\text{Expr}]  & {prec} = 0\\
        [\text{Expr}] < [\text{Expr}]  & {prec} = 0\\
        [\text{Expr}] >= [\text{Expr}] & {prec} = 0\\
        [\text{Expr}] <= [\text{Expr}] & {prec} = 0\\
    \end{cases} \\

    [\text{Term}] &\to 
    \begin{cases}
        \text{identifier} \\
        [\text{FunctionCall}] \\
        [\text{boolean\_literal}] \\
        \text{integer\_literal} \\
        '\text{char\_literal}' \\
        "[\text{string\_literal}]" \\
        ([\text{Expr}])
    \end{cases} \\

    [\text{boolean\_literal}] &\to
    \begin{cases}
        true \\
        false
    \end{cases} \\

    [\text{string\_literal}] &\to [\text{char\_literal}]^* \\

    [\text{Type}] &\to
    \begin{cases}
        bool \\
        int \\
        char \\
        string \\
    \end{cases} \\

\end{aligned}

$$
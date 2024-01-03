$$

\begin{align}

[\text{prog}] &\to [\text{stmt}]^* \\

[\text{stmt}] &\to 
\begin{cases}
    \text{return [expr]} \\
    \text{let identifier} = [\text{expr}]
\end{cases} \\

[\text{expr}] &\to 
\begin{cases}
    \text{integer\_literal} \\
    \text{identifier}
\end{cases} \\

\end{align}

$$
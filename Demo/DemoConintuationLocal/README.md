# DemoConintuationLocal

A static browser demo comparing the local continuation control forms in Rho, and the Pi each one becomes.

| Rho | Pi | What happens |
|-----|----|--------------|
| `f(a)`, `f(a)&` | `Suspend` | Push a return point; control comes back to the caller when the callee finishes |
| `f(a)...` | `Resume` | Switch to the callee without pushing a return point; the context stack is left as it is |
| `f(a)!` | `Replace` | Replace the current continuation with the callee, dropping the top context entry (a tail call) |

```mermaid
sequenceDiagram
    participant C as Caller
    participant F as f
    Note over C,F: f(a) / f(a)&  (Suspend)
    C->>F: push return point, enter f
    F-->>C: return to caller
    Note over C,F: f(a)!  (Replace)
    C->>F: f replaces the caller, nothing to return to
```

Sigma supports `&` and `!` with the same meaning; `...` is not yet available from Sigma. See [Sigma: continuation operators](../../Doc/Sigma/README.md#continuation-operators).

Open `index.html` in a browser. The page reuses `../ContinuationMobilityDemo/style.css`, shared styling from `../../SharedWeb/styles/kai-shared.css`, and its own `app.js`.

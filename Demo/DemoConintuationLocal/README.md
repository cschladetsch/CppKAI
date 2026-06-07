# DemoConintuationLocal

`DemoConintuationLocal` is a static browser demo for comparing local continuation
control forms in Rho and their Pi shape.

- `f(a)` / `f(a)&`: suspend the current continuation, push a return point, and
  return to the caller when the callee completes.
- `f(a)...`: resume/switch to the callee without pushing a new return point.
  The existing context stack is left intact.
- `f(a)!`: replace the current continuation with the callee and drop the current
  top context stack entry.

Open `index.html` in a browser. The demo reuses
`../ContinuationMobilityDemo/style.css`, shared KAI web styling from
`../../SharedWeb/styles/kai-shared.css`, and local page-specific behavior in
`app.js`.

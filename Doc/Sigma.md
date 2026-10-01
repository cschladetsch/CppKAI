# Sigma Language Documentation

## Postfix Operators in Calls

Sigma supports three primary postfix operators on expressions and calls, mirroring the syntax conventions used in Rho:

### 1. `!` (Force / Assert / Execute)
* **Syntax:** `expr!`
* **Semantics:** Forces immediate evaluation, asserts non-null/success, or evaluates a continuation eagerly depending on the context. Used to strip optionality or trigger immediate execution of deferred blocks.

### 2. `...` (Spread / Variadic Expansion)
* **Syntax:** `expr...`
* **Semantics:** Spreads a collection, tuple, or argument list inline. Used in function calls to expand a sequence type into individual arguments, or in container definitions for variadic unpacking.

### 3. `&` (Reference / Borrow / Continuation Capture)
* **Syntax:** `expr&`
* **Semantics:** Captures an expression or call result by reference, or creates a lazy reference/continuation wrapper rather than evaluating immediately by value.

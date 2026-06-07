# Rho, Pi, and Tau Correction

KAI language assistance should be available at the CppKAI layer for consumers.
Consumers should not need application-specific code to ask for Rho, Pi, or Tau
repair suggestions.

## Rho

Rho is an infix language that transpiles to Pi. When a user writes a broken
pattern such as:

```rho
foo fum(a,b) {
    return a + b
}
```

the assistant should infer whether the user intended a function declaration,
identify the syntax error, and propose a valid Rho form that preserves intent.
The correction should include the resulting Pi when that helps verification.

## Pi

Pi is stack-oriented. Assistance should reason about stack effects. For broken
Pi input, explain the expected stack state before and after the expression and
avoid corrections that silently consume or produce the wrong number of values.

## Tau

Tau is an interface definition language. Assistance should preserve interface,
method, and type intent while correcting namespace, declaration, or generation
syntax. Tau corrections should be suitable for agent/proxy generation.

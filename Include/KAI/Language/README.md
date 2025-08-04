# Language

This module defines a set of Languages.

Each language is in it's own static library, but they share a common core system.

Specifically, each language (Pi, Rho Tau) are all implemented using the same core language syetms defined in KAI/Language/Common.

## Pi

Post-fix notation with two stacks: one for data, one for context. Like Forth.

## Rho

In-fix notation. Translated to Pi code. Looks like Python.

## Tau

An interface definition language (IDL) used to generate network agent and proxies.


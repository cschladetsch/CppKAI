# Language

This library defines the three key languages used by the system:
* **Pi** A post-fix language inspired by forth.
* **Rho** An in-fix language inspired by Python. Complies to Pi.
* **Tau** An Interface Definitional Language (_IDL_) that generates C++ code for both consumers and agents of a given network entity.

These three languages are supported by a language-agnostic lexer/parser/ast system that is used by all three systems. An attempt is underway to extract this common language system out of other dependancies of the system, called [FPL](https://github.com/cschladetsch/fpl).

## Pi

_Pi_ is a simple post-fix language with two stacks: A data- and a context-stack.

## Rho

_Rho_ is a simple in-fix dynamically typed language. It smells a lot like Python, but has native co-routine support.

## Tau

_Tau_ is an Interface Definition Language (_IDL_). It is used to generate both consumer _Proxy_ and producer _Agent_ code.

A single Network entity begins with a definition in the Tau language. From there, Proxies are used to access Agents.

# Summary

There are currently three different language systems used by the system:
- Pi
- Rho
- Tau

The first two are used to access C++ systems reflected at runtime. 

The last, Tau, is used as an IDL to generate Proxy and Agent code
that can be statically linked to executables that interact via a Node 
system.

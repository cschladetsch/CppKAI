# KAI Project Overview

## CppKAI (Core Runtime)
CppKAI is the foundational C++ repository implementing a multi-language runtime system. It includes:
- **Core Engine**: Object system, garbage collection, and stack-based executor.
- **Languages**: Interpreters for Pi, Rho, Tau, Lisp, and Hlsl.
- **Networking**: Peer-to-peer networking capabilities.
- **Tooling**: Debugging, logging, and LLM-assisted code analysis tools.

## KAIKaspar (Android Application)
KAIKaspar is an Android-based application (located in `/Android/KaiKaspar`) designed to leverage the CppKAI runtime within a mobile environment. It utilizes JNI (Java Native Interface) to bridge the Android UI with the core C++ engine.

## Relationship
CppKAI serves as the core library and runtime, while KAIKaspar acts as a platform-specific application client. Development workflow typically involves maintaining the CppKAI core and exposing functionality to KAIKaspar via JNI wrappers.

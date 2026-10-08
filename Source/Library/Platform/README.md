# Platform

Platform-specific sources. Very little lives here: the code has been cross-platform from the start, and platform differences are mostly handled in headers under [`Include/KAI/Platform`](../../../Include/KAI/Platform/README.md).

| Directory | Contents |
|-----------|----------|
| `Windows/` | `ConsoleColors.cpp`: console colour support |
| `GearVr/`, `Hololense/`, `PS4/`, `Rift/` | Placeholders, no code |

The CMake and header structure is already in place for platform libraries (`platform-<name>`), intended for hardware such as VR and AR headsets and game controllers.

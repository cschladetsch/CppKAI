# Include

Public headers built in CppKAI. Most of KAI's headers come from the submodules, which add their own `Include` directories to the path, so `#include <KAI/...>` works the same either way.

```mermaid
flowchart TB
    subgraph Here["Include/KAI (this repo)"]
        L["Language/<br/>Sigma, PiNet, Tau, Lisp, Hlsl"]
        N["Network/"]
        LLM["LLM/"]
        P["Platform/"]
    end
    subgraph Core["Ext/CppKaiCore/Include/KAI"]
        C["Core/, Executor/,<br/>Language/Common, KAI.h"]
    end
    subgraph Lang["Ext/CppKaiLanguage/Include/KAI"]
        PR["Language/Pi, Language/Rho"]
    end
    subgraph CL["Ext/CppKaiConsoleLib/Include/KAI"]
        CO["Console/"]
    end
    Here --> Core
    PR --> Core
    CO --> PR
```

| Area | Where | What |
|------|-------|------|
| Core | `Ext/CppKaiCore` | Objects, Registry, Tree, types, reflection, tri-colour GC |
| Executor | `Ext/CppKaiCore` | Two-stack virtual machine (data and context), inspired by Forth |
| Pi, Rho | `Ext/CppKaiLanguage` | Postfix and infix languages |
| Console | `Ext/CppKaiConsoleLib` | Cross-platform REPL for Pi, Rho and Sigma; switch language on the fly |
| Sigma, PiNet, Tau | [`KAI/Language`](KAI/Language/README.md) | Typed language, send check, IDL |
| Network | `KAI/Network` | Node, Domain, Agent, Proxy, `Future<T>`. Independent of transport; ENet is the current implementation |
| LLM | `KAI/LLM` | Model cache, session, repo indexer, dataset builder |
| Platform | [`KAI/Platform`](KAI/Platform/README.md) | Platform-specific headers |

`kai_compat.h` and `rang.hpp` (terminal colours) are also here.

See [KAI/README.md](KAI/README.md) for the header-level tour.

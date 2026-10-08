# Source

```mermaid
flowchart TB
    subgraph App["App/"]
        CON[Console]
        WIN["Window (ImGui)"]
        RI[RepoIndex]
        RD[RhoDataset]
    end
    subgraph Library["Library/"]
        LANG["Language<br/>Sigma, PiNet, Tau"]
        NET[Network]
        LLM[LLM]
        PLAT[Platform]
    end
    CON --> LANG & NET
    WIN --> LANG
    RI & RD --> LLM
```

- **[App](App/README.md)**: executables built on KAI: Console, Window (ImGui), RepoIndex and RhoDataset
- **[Library](Library/README.md)**: the libraries built in this repo. Core, Executor, `Language/Common`, Pi and Rho come from the `Ext/CppKaiCore` and `Ext/CppKaiLanguage` submodules

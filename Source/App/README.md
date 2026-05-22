# KAI Applications

There are are a number of applications of KAI. This folder contains all of the root applications, including:

1. **Console**. A basic colored console interface to an Executor.
1. **Window**. New GUI console based on ImGui. Build it with `-DKAI_BUILD_WINDOW=ON`.
1. **RepoIndex**. Builds a local code/test knowledge base for LLM-assisted retrieval. Build it with `-DKAI_BUILD_LLM=ON`. It writes under the same `~/.cache/deepseek/models` model cache used by `Ext/CppLmmModelStore`.
1. **NetworkGen**. Used to generate _Agents_ and _Proxies_ for networking.
1. **NetworkPeer**. More advanced: will be a way to connect and communicate with other peers.
1. **Deprecated**. Apps that no longer serve a purpose but are kept around for reference.

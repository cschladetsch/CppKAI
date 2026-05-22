# RepoIndex

RepoIndex scans the KAI repository and builds a local chunked knowledge base for
assistant-style retrieval.

By default it writes to the local model cache from `CppLmmModelStore`:

```bash
./Bin/RepoIndex
```

Override the source root or output root if you want to index a different tree:

```bash
./Bin/RepoIndex --root /path/to/KAI --out /tmp/kai-index
```

# LLM Notes for KAI

KAI’s LLM support is deliberately narrow:

- `RepoIndex` builds a local code/test knowledge base for retrieval.
- `RhoDataset` exports Rho-focused training data as JSONL.
- `LLM::ModelCache` resolves the cache root and model paths.
- `LLM::Session` is the seam for a future llama.cpp backend.

## Default cache

Models and generated artifacts default to:

```text
~/.cache/deepseek/models
```

That path comes from `Ext/CppLmmModelStore` through `LLM::ModelCache`.

## Useful commands

```bash
./Bin/RepoIndex
./Bin/RhoDataset
./Bin/RhoDataset --root . --out /tmp/kai-rho-training
```

## Current scope

What exists now:

- cache/path management
- repo indexing for retrieval
- Rho dataset export for future fine-tuning
- a minimal session interface for backend integration

What is not in-tree yet:

- a real llama.cpp runtime binding
- model training inside KAI
- prompt orchestration beyond the `LLM::Session` seam

The intended workflow is:

1. Build the repo index for grounding.
2. Export a Rho dataset from repo-local sources.
3. Train externally with LoRA or a similar method.
4. Put the resulting model under the cache tree.
5. Load it through `LLM::Session`.

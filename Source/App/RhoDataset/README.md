# RhoDataset

RhoDataset is the compatibility name for the local KAI language dataset builder.
It writes JSONL records for Rho, Pi, Tau, gtest evidence, logs, history files,
README documentation, and incremental lessons from `Scripts/Training` under the
same cache tree used by the other LLM tools.

This tool builds corpus records. It does not imply that KAI already contains
trained weights. The output can be used for retrieval, supervised examples,
fine-tuning, or any later llama.cpp/CppLmmModelStore training pipeline.

By default it should ingest the local corpus silently. Human confirmation is
only appropriate when a proposed addition would have a large impact on scope or
provenance.

Run it with the default cache location:

```bash
./Bin/RhoDataset
```

Override the source root or output root if you want to generate a dataset for
another tree:

```bash
./Bin/RhoDataset --root /path/to/KAI --out /tmp/kai-language-training
```

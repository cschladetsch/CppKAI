# RhoDataset

RhoDataset builds a local JSONL training set for Rho-focused model work.
It pulls examples from repo-local Rho tests, scripts, and documentation and
writes them under the same cache tree used by the other LLM tools.

Run it with the default cache location:

```bash
./Bin/RhoDataset
```

Override the source root or output root if you want to generate a dataset for
another tree:

```bash
./Bin/RhoDataset --root /path/to/KAI --out /tmp/kai-rho-training
```

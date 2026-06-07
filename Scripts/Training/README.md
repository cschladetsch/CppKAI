# KAI Incremental Training Corpus

This folder contains markdown lessons used by the local LLM dataset builder to
teach KAI about itself. These files are source material for incremental
training, retrieval, prompt construction, or supervised examples. They are not
evidence that the repository already contains trained model weights.

The default policy is silent ingestion: KAI should add ordinary corpus material
without asking. It should ask only when a proposed addition would have a large
impact on the training set, such as a new external corpus, a broad rewrite of
curated examples, or a provenance change that materially alters what the model
will learn.

`./Bin/RhoDataset` includes this folder together with:

- Rho, Pi, and Tau language tests
- gtest files that encode expected behavior
- `Logs/` runtime and failure evidence
- `.history` console sessions
- README and selected documentation files
- console scripts and language examples

Use one focused topic per markdown file. Prefer short examples with the
expected interpretation, failure mode, and repair strategy.

## Suggested Record Shape

Each lesson should answer:

- what concept the model should learn
- which language or subsystem it applies to
- what valid input looks like
- what common broken input looks like
- how KAI should correct, explain, or route the input

The dataset builder emits these files as `kind: "doc"` with `language: "KAI"`
and an instruction that marks them as incremental training notes.

## Training Memory

`RhoDataset` writes a `manifest.json` next to `dataset.jsonl`. The manifest is
the durable memory of what was ingested in a given run. Keep it alongside the
dataset output so the corpus can be audited, repeated, or compared later.

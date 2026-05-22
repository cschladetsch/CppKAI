# Rho Model Training Plan

This is the lowest-friction path to train a small model that is useful for Rho work in KAI.

## Goal

Produce a model that can:
- explain Rho syntax and control flow
- translate small Rho snippets into Pi
- repair common Rho errors using the repo's existing tests as ground truth
- generate small Rho examples that compile and run in KAI

## Data Sources

Use only repo-local sources first:
- `Test/Language/TestRho`
- `Test/Language/TestPi`
- `Test/Examples`
- `Source/App/Console`
- `Doc/RhoTutorial.md`
- `Doc/PiTutorial.md`
- `Doc/Console.md`

Prioritize:
- passing test cases
- small focused examples
- snippets that show one feature at a time
- failing tests with known fixes, if the failure and fix are both documented

## Dataset Shape

Create instruction-style JSONL records:

- `{"instruction":"Write Rho for a loop that sums 1..10","input":"","output":"..."}`
- `{"instruction":"Explain this Rho snippet","input":"...","output":"..."}`
- `{"instruction":"Translate this Rho to Pi","input":"...","output":"..."}`
- `{"instruction":"Fix this Rho test failure","input":"...","output":"..."}`

Keep examples short and direct. Prefer one behavior per record.

## Training Approach

Use llama.cpp as the runtime target, but train externally with LoRA:

1. Build the dataset from the repo.
2. Fine-tune a small base model with LoRA using an external training pipeline.
3. Export the merged or adapter weights.
4. Place the model under `~/.cache/deepseek/models` via `Ext/CppLmmModelStore`.
5. Load it from KAI through `LLM::Session`.

## Evaluation

Measure against repo-native tasks:
- Rho parsing and translation examples
- Rho-related unit tests
- simple Pi translation tasks
- small code generation prompts

Pass criteria:
- the model generates valid Rho more often than the base model
- it produces fewer syntax errors on the known examples
- it does not regress on Pi translation or Rho explanations

## Integration

The KAI-facing integration point is:
- `LLM::ModelCache` for locating the model
- `LLM::Session` for loading and prompting
- `RepoIndex` for grounding prompts in the repository

The actual llama.cpp inference backend can be added later without changing those entry points.


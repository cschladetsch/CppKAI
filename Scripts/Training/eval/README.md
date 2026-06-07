# LLM Evaluation Suite

This folder holds prompt suites used to test a real model after the corpus has
been built. The suite is part of the local training memory and is intended to be
versioned alongside `Scripts/Training/`.

Use it to check:

- Rho syntax repair
- Pi stack-effect reasoning
- Tau interface correction
- the quality of outputs after a training or fine-tuning pass

The repository does not include an in-tree llama.cpp runtime binding yet. The
test harness has two modes:

- a deterministic fallback that validates the evaluator and prompt suite
- a real-model path that reads a model-backed command from `KAI_LLM_EVAL_COMMAND`

The real-model path skips cleanly if the command is not configured.

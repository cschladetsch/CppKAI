# Executor and Log Evidence

The Executor has a data stack and a context stack. Language assistance should
use both stacks when explaining runtime behavior or proposing repairs.

## Data Stack

The top of stack is index `0`. Deeper values are shown with increasing indexes:

```text
[2] V2
[1] V1
[0] V0
```

When repairing Pi, the assistant should describe how each token changes the data
stack and whether the final stack shape matches the requested operation.

## Context Stack

The context stack holds continuations and execution state. When a continuation
is frozen, moved, thawed, and resumed, assistance should explain which context
items are preserved and which node-local resources must be rebound.

## Logs

Files under `Logs/` are training evidence. They teach the assistant about real
parser errors, runtime failures, test regressions, and expected diagnostic
language. Log-derived corrections should cite the observed failure pattern and
prefer fixes already represented in tests or scripts.

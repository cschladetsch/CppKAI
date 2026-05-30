# ContinuationMobilityDemo Plan

This directory now serves as a companion bundle for the single-file Rho demo:
[ContinuationMobilityDemo.rho](./ContinuationMobilityDemo.rho).

The intent is to show one scenario clearly:

- mobile agents moving between logical hosts
- Pi-guided routing decisions
- host load rebalancing
- snapshot-based recovery after a simulated host failure

The original draft described this in terms of a richer continuation runtime
with concepts like suspend, serialize, restore, and resume crossing host
boundaries. That remains the conceptual story. The current implementation is
more concrete and intentionally easier to read:

- the Rho script is the canonical narrative
- the HTML pages are visual explanations of that narrative
- the C++ executable is a reference simulation, not the source of truth
- recovery is modeled from captured agent snapshots
- all behavior is deterministic and bounded

## Current Scenario

The scenario uses four regions:

- `Start` on `NodeA`
- `Market` on `NodeB`
- `Harbor` on `NodeC`
- `Backup` on `NodeD`

It creates ten agents and walks through the key milestones that matter for the
demo:

- agents start distributed across the three live regions
- Pi reroutes the `Market` crowd to `Harbor`
- snapshots capture the full agent state
- the balancer shifts load to `Backup`
- a deterministic drill forces agents `1`, `2`, and `3` onto `NodeB`
- `NodeB` fails and those agents are restored from snapshot onto `NodeD`

## Tests

Automated coverage now lives in two places:

- [ContinuationMobilityDemoScriptTest.cpp](../../Test/Language/TestRho/ContinuationMobilityDemoScriptTest.cpp)
  verifies that the Rho source file executes successfully
- [ContinuationMobilityDemoTests.cpp](../../Test/Examples/ContinuationMobilityDemoTests.cpp)
  verifies the reference executable model

## Future Direction

If the KAI runtime eventually grows real continuation migration hooks, the next
step would be to make the Rho story drive those hooks directly. Until then, the
demo should be read as a clear executable model of the intended behavior rather
than as proof that full cross-host continuation migration already exists.

# ContinuationMobilityDemo

`ContinuationMobilityDemo` is a documentation and visualization bundle for the
single-file Rho demo at
[ContinuationMobilityDemo.rho](./ContinuationMobilityDemo.rho).

That Rho script is the source of truth. Everything else in this folder exists to
make the same scenario easier to inspect:

- the C++ executable is a reference simulation of the same story
- `ContinuationMobilityDemo.html` is the system-level explanation
- `ContinuationMobilityAnimation.html` is the migration-focused explanation
- `Scripts/network/run_continuation_migration_demo.sh` is the two-process
  runtime proof that a frozen Pi continuation can move across the network

It demonstrates:

- mobile agents moving between regions hosted on different logical nodes
- Pi-style planning that can redirect agents based on local conditions
- host load tracking and periodic rebalancing
- snapshot capture of agent state
- simulated node failure and recovery onto a backup host
- a deterministic failure drill at tick `60` so recovery is exercised every run
- visible logging for every major event

## What This Proves

KAI can treat computation as data.

At the model level, agents carry state, move between hosts, survive failure, and
resume from snapshots. At the runtime level, a stateful Pi workflow can be
frozen with `step=3`, `accumulator=21`, and remaining work, sent to another
process, thawed, resumed, and returned as a normal typed result: `42`.

This is useful for:

- resumable distributed workflows
- live migration of computation
- failure recovery without restarting work
- agent systems where behavior and state move together

## Proof Stack

The demo is easiest to understand as three layers:

1. Conceptual view: the RhoMog model in this directory explains the distributed
   agent story in human-readable form.
2. Executable model: `./Bin/ContinuationMobilityDemo` runs a deterministic C++
   simulation of the same migration, balancing, snapshot, and recovery flow.
3. Runtime proof: `./Scripts/network/run_continuation_migration_demo.sh` starts
   two processes, freezes a stateful Pi continuation in one process, sends it
   to another process, thaws it, resumes it, and verifies the result.

What changed from the original sketch:

- removed references to non-existent APIs such as `Runtime`, `ObjectRef`,
  `Reflect`, `RestoreObject`, and `YieldTo`
- replaced the fantasy migration path with an explicit state transfer model
- fixed the recovery model so restored agents do not get duplicated endlessly
- fixed the host transition semantics so migration changes agent placement, not
  region ownership
- moved the canonical narrative into a single Rho file with no cross-file
  symbols
- kept the executable model deterministic and finite so it can be built and run
  in CI or from the command line

## Build

From the repo root:

```bash
cmake --build ./build --target ContinuationMobilityDemo
```

## Test

Smoke test for the Rho source of truth:

```bash
cmake --build ./build --target TestRho
./Bin/Test/TestRho '--gtest_filter=ContinuationMobilityDemoScriptTest.*'
```

Reference executable test suite:

```bash
cmake --build ./build --target ContinuationMobilityDemoTests
./build/Bin/Test/Examples/ContinuationMobilityDemoTests
```

Browser smoke test for the HTML workbench:

```bash
git submodule update --init --recursive Demo/ContinuationMobilityDemo/ThirdParty/playwright
cd Demo/ContinuationMobilityDemo/ThirdParty/playwright
npm ci
npm run build
cd ../..
node ThirdParty/playwright/packages/playwright-core/cli.js install chromium
node test_workbench_playwright.js
```

## Run

```bash
./Bin/ContinuationMobilityDemo
```

## Run the Proofs in Order

From the repository root:

```bash
# 1. Run the readable simulation
cmake --build ./build --target ContinuationMobilityDemo
./Bin/ContinuationMobilityDemo

# 2. Run the two-process runtime proof
./Scripts/network/run_continuation_migration_demo.sh

# 3. Run the regression tests
cmake --build ./build --target ContinuationMobilityDemoTests
./build/Bin/Test/Examples/ContinuationMobilityDemoTests
./Bin/Test/TestRho '--gtest_filter=ContinuationMobilityDemoScriptTest.*'
```

## Output

The Rho script prints:

- a clear summary of the initial world state
- Pi re-routing from `Market` to `Harbor`
- snapshot capture before failure
- load balancing to `Backup`
- the failure drill that moves agents `1`, `2`, and `3` onto `Market`
- recovery of those agents onto `NodeD`
- a final count of restored agents

The executable model prints a more detailed tick-by-tick log, but it is not the
canonical description of the demo anymore.

This is a simulation, not a real distributed runtime. Its purpose is to make
the migration / load-balancing / recovery story concrete and inspectable inside
the current repository.

For the actual network freeze/thaw path, run
`./Scripts/network/run_continuation_migration_demo.sh`.

Expected success markers:

```text
CLIENT_WORKFLOW_STATE step=3 accumulator=21 remaining=[4,5,6,6]
SERVER_RESUMED_STATEFUL_WORKFLOW final_step=7 final_accumulator=42
CLIENT_RESULT=42
MIGRATION_OK result=42
```

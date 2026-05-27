# ContinuationMobilityDemo

`ContinuationMobilityDemo` is a documentation and visualization bundle for the
single-file Rho demo at
[ContinuationMobilityDemo.rho](./ContinuationMobilityDemo.rho).

That Rho script is the source of truth. Everything else in this folder exists to
make the same scenario easier to inspect:

- the C++ executable is a reference simulation of the same story
- `ContinuationMobilityDemo.html` is the system-level explanation
- `ContinuationMobilityAnimation.html` is the migration-focused explanation

It demonstrates:

- mobile agents moving between regions hosted on different logical nodes
- Pi-style planning that can redirect agents based on local conditions
- host load tracking and periodic rebalancing
- snapshot capture of agent state
- simulated node failure and recovery onto a backup host
- a deterministic failure drill at tick `60` so recovery is exercised every run
- visible logging for every major event

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

## Run

```bash
./Bin/ContinuationMobilityDemo
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

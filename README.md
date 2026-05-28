# BoddTest

A small CMake C++ project that uses Boost in production code and GoogleTest for tests.

## Build

```sh
cmake -S . -B build
cmake --build build
```

## Test

```sh
ctest --test-dir build --output-on-failure
```

## Run

```sh
./build/bodd "hello boost and gtest"
```

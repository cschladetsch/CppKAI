# ksh (KAI Object Shell)

A next-generation execution environment shell built on C++23 and KAI, targeting Clang.

## Building with Clang

```bash
mkdir build && cd build
cmake -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ ..
cmake --build .
ctest --output-on-failure
./ksh
```

#!/bin/bash

mkdir -p build && cd build
cmake -G Ninja ..

# Check if target parameter is provided
if [ "$1" != "" ]; then
  # Build specific target
  ninja "$1"
else
  # Build all targets
  ninja
fi

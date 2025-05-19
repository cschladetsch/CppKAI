# Simple Makefile for KAI project
# Uses clang++ by default, can be overridden with make CXX=g++

.PHONY: all clean build test

# Default to clang++
CXX ?= clang++

BUILD_DIR := build
BIN_DIR := $(BUILD_DIR)/Bin

all: build

build:
	mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && \
	cmake .. \
		-DCMAKE_CXX_COMPILER=$(CXX) \
		-DCMAKE_BUILD_TYPE=Debug \
		-DCMAKE_RUNTIME_OUTPUT_DIRECTORY="${PWD}/$(BUILD_DIR)/Bin" \
		-DCMAKE_LIBRARY_OUTPUT_DIRECTORY="${PWD}/$(BUILD_DIR)/Bin" \
		-DCMAKE_ARCHIVE_OUTPUT_DIRECTORY="${PWD}/$(BUILD_DIR)/Bin" \
		-DBIN_HOME="${PWD}/$(BUILD_DIR)/Bin" \
		-DKAI_BUILD_TEST_ALL=OFF && \
	cmake --build .

gcc:
	$(MAKE) CXX=g++

clang:
	$(MAKE) CXX=clang++

clean:
	rm -rf $(BUILD_DIR)

test: build
	cd $(BUILD_DIR) && ctest -V
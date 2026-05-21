#!/usr/bin/env python3
import sys

TARGET = "/home/christian/local/repos/KAI/Test/Network/NodeEndToEndTest.cpp"

OLD = """\
    auto cont = clientConsole.Compile(
        "{ dup 0 == { drop 0 } { dup 1 - recurse + } if } 'sum # 5 sum &",
        Structure::Program);

    Object frozen = Bin::Freeze(*cont->Self);
    BinaryStream bs;
    try {
        bs << frozen;
    } catch (const Exception::Base &e) {
        FAIL() << "Failed to serialize frozen continuation: " << e.ToString();
    }

    auto future = client.Invoke<int>(agentHandle, "ThawAndResume", bs);\
"""

NEW = """\
    std::cerr << "[DIAG] Compiling\\n";
    auto cont = clientConsole.Compile(
        "{ dup 0 == { drop 0 } { dup 1 - recurse + } if } 'sum # 5 sum &",
        Structure::Program);
    std::cerr << "[DIAG] Compiled OK\\n";

    std::cerr << "[DIAG] Freezing\\n";
    Object frozen = Bin::Freeze(*cont->Self);
    std::cerr << "[DIAG] Frozen OK\\n";
    BinaryStream bs;
    try {
        std::cerr << "[DIAG] Serialising\\n";
        bs << frozen;
        std::cerr << "[DIAG] Serialised OK\\n";
    } catch (const Exception::Base &e) {
        FAIL() << "Failed to serialize frozen continuation: " << e.ToString();
    }

    std::cerr << "[DIAG] Invoking\\n";
    auto future = client.Invoke<int>(agentHandle, "ThawAndResume", bs);\
"""

with open(TARGET, 'r') as f:
    src = f.read()

if OLD not in src:
    print("ERROR: target block not found", file=sys.stderr)
    sys.exit(1)

if "[DIAG]" in src:
    print("Diagnostics already present.")
    sys.exit(0)

with open(TARGET, 'w') as f:
    f.write(src.replace(OLD, NEW, 1))

print(f"Patched {TARGET}")

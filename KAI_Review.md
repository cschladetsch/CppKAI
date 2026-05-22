# KAI Review

## Summary

KAI has a strong shape: the reflected object model, registry-driven allocation, and the continuation/network work all fit the project’s goals. The main risks I found are in the binary serialization boundary, where malformed data or internal errors can currently collapse into silent success.

## Findings

1. **High - malformed network payloads can trigger oversized allocation**
   [Include/KAI/Network/Serialization.h](/home/christian/local/repos/KAI/Include/KAI/Network/Serialization.h:74) reads a signed `int size` from the packet, but only rejects `size <= 0` after the read. A negative value is still converted to `std::size_t` at line 84 and used to allocate `std::vector<char> buffer(...)`, which can turn a corrupt or hostile packet into a huge allocation attempt. The guard needs to reject negative sizes before allocation.

2. **High - serialization failures are silently converted into null objects**
   [Include/KAI/Network/Serialization.h](/home/christian/local/repos/KAI/Include/KAI/Network/Serialization.h:18) catches every exception during `SerializeObject`, writes a zero length, and returns. That makes “failed to serialize” indistinguishable from “legitimate null object” on the wire. In practice this hides bugs and data corruption, because the receiver cannot tell whether a null payload was intentional or an error fallback.

3. **Medium - binary object serialization silently truncates deep graphs**
   [Source/Library/Core/Source/Object.cpp](/home/christian/local/repos/KAI/Source/Library/Core/Source/Object.cpp:398) imposes `MAX_SERIALIZATION_DEPTH = 10` and replaces deeper objects with a null marker. That avoids recursion, but it also means valid object graphs deeper than ten levels are lossily serialized without any error. For continuations and nested object graphs, this can drop data in a way that is hard to diagnose.

## Notes

- The stray `[THAW]` debug trace in `Object.cpp` has been removed.
- I did not see a crash in the current build path, but the network serializer still needs stronger input validation if it is expected to handle untrusted packets.

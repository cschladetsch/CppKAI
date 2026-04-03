#pragma once

#include <KAI/Core/Config/Base.h>

#include <unordered_set>

KAI_BEGIN

class Handle {
   public:
    typedef int Value;

   private:
    Value value;

   public:
    explicit Handle(Value V = 0) : value(V) {}

    Value GetValue() const { return value; }
    Value NextValue() { return ++value; }

    friend bool operator<(Handle A, Handle B) { return A.value < B.value; }
    friend bool operator==(Handle A, Handle B) { return A.value == B.value; }
    friend bool operator!=(Handle A, Handle B) { return A.value != B.value; }
    operator bool() const { return GetValue(); }

    static void Register(Registry &);
};

struct HashHandle {
    enum { bucket_size = 8, min_buckets = 2048 };
    std::size_t operator()(const Handle &A) const { return A.GetValue(); }

    bool operator()(const Handle &A, const Handle &B) const {
        return A.GetValue() < B.GetValue();
    }
};

StringStream &operator<<(StringStream &, Handle);

typedef std::unordered_set<Handle, HashHandle> HandleSet;

KAI_END

#pragma once

#include <KAI/Core/Config/Base.h>
#include <KAI/Core/Object/Object.h>
#include <KAI/Core/TriColor.h>

#include <unordered_set>

#include "Container.h"

KAI_BEGIN

struct CompareHandles {
    bool operator()(Object const &A, Object const &B) const {
        return A.GetHandle() == B.GetHandle();
    }
};

struct HashObject {
    inline size_t operator()(Object const &A) const {
        return A.GetHandle().GetValue();
    }
};

// I'm sure there was a good reason I named this ObjectSet rather than just Set.
// And at this point, I am too afraid to ask.
struct ObjectSet : Container<ObjectSet> {
    typedef std::unordered_set<Object, HashObject, CompareHandles> Objects;
    typedef Objects::const_iterator const_iterator;
    typedef Objects::iterator iterator;

   private:
    Objects objects;

   public:
    bool Destroy();

    const_iterator begin() const { return objects.begin(); }
    const_iterator end() const { return objects.end(); }
    iterator begin() { return objects.begin(); }
    iterator end() { return objects.end(); }

    const_iterator Begin() const { return objects.begin(); }
    const_iterator End() const { return objects.end(); }
    iterator Begin() { return objects.begin(); }
    iterator End() { return objects.end(); }

    int Size() const;
    bool Empty() const;
    void Clear();
    iterator Erase(iterator);
    iterator Erase(Object const &);
    void Append(Object const &);
    void Insert(Object const &);
    void Remove(Object const &);
    bool Contains(Object const &);

    void SetChildSwitch(int N, bool M) {
        ForEach(objects, SetSwitch<ObjectSet>(N, M));
    }

    static void Register(Registry &);
};

StringStream &operator<<(StringStream &, ObjectSet const &);
StringStream &operator>>(StringStream &, ObjectSet &);

BinaryStream &operator<<(BinaryStream &, ObjectSet const &);
BinaryStream &operator>>(BinaryStream &, ObjectSet &);

KAI_TYPE_TRAITS(ObjectSet, Number::Set,
                Properties::StringStreamInsert | Properties::BinaryStreaming |
                    Properties::Reflected | Properties::Container);

KAI_END

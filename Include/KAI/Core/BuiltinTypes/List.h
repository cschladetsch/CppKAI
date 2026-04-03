#pragma once

#include <KAI/Core/Object/Object.h>

#include <list>

#include "KAI/Core/BuiltinTypes/Container.h"

KAI_BEGIN

/// A list of Objects
struct List : Container<List> {
    typedef std::list<Object> Objects;
    typedef Objects::const_iterator const_iterator;
    typedef Objects::iterator iterator;

   private:
    Objects objects;

   public:
    bool Destroy() {
        Clear();
        return true;
    }

    iterator begin() { return objects.begin(); }
    iterator end() { return objects.end(); }
    const_iterator begin() const { return objects.begin(); }
    const_iterator end() const { return objects.end(); }

    iterator Begin() { return objects.begin(); }
    iterator End() { return objects.end(); }
    const_iterator Begin() const { return objects.begin(); }
    const_iterator End() const { return objects.end(); }
    int Size() const { return (int)objects.size(); }
    bool Empty() const { return objects.empty(); }
    Object Front() const { return objects.front(); }
    Object Back() const { return objects.back(); }

    void Clear();

    void Append(Object const &Q);
    void PushBack(Object const &Q) { Append(Q); }
    Object Pop();
    Object PopBack() { return Pop(); }
    iterator Erase(iterator A);
    iterator Erase(Object const &Q);
    bool Contains(Object const &Q) const {
        for (auto const &element : objects) {
            if (element.GetHandle() == Q.GetHandle()) return true;
        }
        return false;
    }

    void Erase2(Object Q) { Erase(Q); }
    void Append2(Object Q) { Append(Q); }
    bool Contains2(Object Q) const { return Contains(Q); }

    friend bool operator==(const List &A, const List &B) {
        return A.objects == B.objects;
    }
    friend bool operator<(const List &A, const List &B) {
        return A.objects < B.objects;
    }

    void SetChildSwitch(int N, bool M) {
        for (auto &elem : objects) elem.SetSwitch(N, M);
    }

    static void Register(Registry &);
};

StringStream &operator<<(StringStream &, const List &);
BinaryStream &operator<<(BinaryStream &, const List &);
BinaryStream &operator>>(BinaryStream &, List &);

HashValue GetHash(const List &A);

KAI_TYPE_TRAITS(List, Number::List,
                Properties::StringStreamInsert | Properties::BinaryStreaming |
                    Properties::Less | Properties::Equiv | Properties::Assign |
                    Properties::Reflected | Properties::Container);

KAI_END

// EOF

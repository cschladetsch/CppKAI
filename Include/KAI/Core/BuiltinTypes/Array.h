#pragma once

#include <KAI/Core/Config/Base.h>

#include "Container.h"

KAI_BEGIN

class Array : public Container<Array> {
   public:
    typedef std::vector<Object> Objects;
    typedef Objects::const_iterator const_iterator;
    typedef Objects::iterator iterator;

   private:
    Objects objects;

   public:
    iterator begin() { return objects.begin(); }
    iterator end() { return objects.end(); }
    const_iterator begin() const { return objects.begin(); }
    const_iterator end() const { return objects.end(); }

    iterator Begin() { return objects.begin(); }
    iterator End() { return objects.end(); }
    const_iterator Begin() const { return objects.begin(); }
    const_iterator End() const { return objects.end(); }

    void Resize(int n) { objects.resize(n); }
    Object At(int pos) const { return objects.at(pos); }
    Object &RefAt(int pos) { return objects.at(pos); }
    int Size() const { return (int)objects.size(); }
    bool Empty() const { return objects.empty(); }
    Object Front() const { return objects.front(); }
    Object Back() const { return objects.back(); }

    void Clear();
    void Append(Object const &);
    void PushBack(Object const &Q) { Append(Q); }
    Object PopBack();
    void RemoveAt(int);
    void Insert(int index, Object const &);

    iterator Erase(iterator A);
    void Erase(Handle A);
    void Erase(Object const &Q);

    void Erase2(Object Q) { Erase(Q); }
    void Append2(Object Q) { Append(Q); }
    void Insert2(int index, Object Q) { Insert(index, Q); }

    friend bool operator==(const Array &A, const Array &B) {
        return A.objects == B.objects;
    }
    friend bool operator<(const Array &A, const Array &B) {
        return A.objects < B.objects;
    }
    friend Array operator+(const Array &A, const Array &B) {
        Array result;
        result.objects = A.objects;
        result.objects.insert(result.objects.end(), B.objects.begin(),
                              B.objects.end());
        return result;
    }

    // void SetChildSwitch(int N, bool M)
    //{
    //     ForEach(objects, SetSwitch<Array>(N, M));
    // }

    static void Register(Registry &);
};

StringStream &operator<<(StringStream &, const Array &);
BinaryStream &operator<<(BinaryStream &, const Array &);
BinaryStream &operator>>(BinaryStream &, Array &);

HashValue GetHash(const Array &A);

KAI_TYPE_TRAITS(Array, Number::Array,
                Properties::StringStreamInsert | Properties::BinaryStreaming |
                    Properties::Less | Properties::Equiv | Properties::Assign |
                    Properties::Reflected | Properties::Container |
                    Properties::Plus);

KAI_END

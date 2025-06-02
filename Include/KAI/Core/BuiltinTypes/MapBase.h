#pragma once

#include <KAI/Core/BuiltinTypes/Signed32.h>
#include <KAI/Core/Exception.h>
#include <KAI/Core/Object/ClassBuilder.h>

#include "Container.h"

KAI_BEGIN

/// A Map defines an associative container. Because this maps Object to Object,
/// it is possible to store mappings of any type to any type.
/// This is the base class for all mappings.
template <class Map>
struct MapBase : Container<Map> {
    typedef typename Map::const_iterator const_iterator;
    typedef typename Map::iterator iterator;
    typedef MapBase<Map> This;

   private:
    Map map;

   public:
    iterator begin() { return map.begin(); }
    iterator end() { return map.end(); }

    const_iterator begin() const { return map.begin(); }
    const_iterator end() const { return map.end(); }

    iterator Begin() { return map.begin(); }
    iterator End() { return map.end(); }
    const_iterator Begin() const { return map.begin(); }
    const_iterator End() const { return map.end(); }
    int Size() const { return (int)map.size(); }
    bool Empty() const { return map.empty(); }
    void Clear() { map.clear(); }
    void Insert(Object const &key, Object const &value) { map[key] = value; }
    bool ContainsKey(Object const &k) const { return Find(k) != End(); }
    const_iterator Find(Object const &k) const { return map.find(k); }

    void Erase(Object const &key) {
        iterator A = map.find(key);
        if (A == map.end()) KAI_THROW_1(UnknownKey, key);
        A->first.SetColor(ObjectColor::White);
        A->second.SetColor(ObjectColor::White);
        map.erase(A);
    }

    Object GetValue(Object const &key) const {
        const_iterator A = map.find(key);
        if (A == map.end()) return Object();  // KAI_THROW_1(UnknownKey, key);
        return A->second;
    }

    void SetChildSwitch(int S, bool M) {
        for (auto const &X : map) {
            const_cast<Object &>(X.first).SetSwitch(S, M);
            X.second.SetSwitch(S, M);
        }
    }

    // friend bool operator<(const This &A, const This &B) { return A.map <
    // B.map; }
    friend bool operator==(const This &A, const This &B) {
        return A.map == B.map;
    }

    static void Register(Registry &R, const char *N) {
        // TODO: Fix const correctness issues with Map methods
        // ClassBuilder<This>(R, Label(N))
        //    .Methods
        //    ("Size", &This::Size)
        //    ("Empty", &This::Empty)
        //    ("Insert", &This::Insert)
        //    ("Erase", &This::Erase)
        //    ("GetValue", &This::GetValue)
        //    ("ContainsKey", &This::ContainsKey)
        //    ;
    }
};

template <class Map>
StringStream &operator<<(StringStream &S, MapBase<Map> const &M) {
    S << "{ ";
    const char *sep = "";
    for (auto const &A : M) {
        S << sep << "[" << A.first << ", " << A.second << "]";
        sep = ", ";
    }
    return S << " }";
}

template <class Map>
StringStream &operator>>(StringStream &S, MapBase<Map> const &M) {
    // S << "{ ";
    // const char *sep = "";
    // for (auto const &A : M)
    // {
    //     S << sep << "[" << A.first << ", " << A.second << "]";
    //     sep = ", ";
    // }

    // this will need a parser....
    KAI_NOT_IMPLEMENTED();
}

template <class Map>
BinaryStream &operator<<(BinaryStream &S, MapBase<Map> const &M) {
    S << M.Size();
    for (const auto &A : M) {
        S << A.first << A.second;
    }
    return S;
}

template <class Map>
BinaryStream &operator>>(BinaryStream &S, MapBase<Map> &M) {
    int length = 0;
    S >> length;
    for (int N = 0; N < length; ++N) {
        Object key, value;
        S >> key >> value;
        M.Insert(key, value);
    }
    return S;
}

KAI_END

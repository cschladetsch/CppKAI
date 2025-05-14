#pragma once

KAI_BEGIN

struct StorageBase;
struct Registry;
template <class T>
struct Pointer;

class Reflected {
   public:
    StorageBase *Self;

    virtual ~Reflected() {}

    virtual void Create() {
    }  // called after object constructed, but before first use
    virtual bool Destroy() {
        return true;
    }  // called when object moved to deathRow_, but before deleted
    virtual void Delete() {
    }  // called immediately before resources are released

    Registry &Reg() const;

    //    template <class T>
    //    Pointer<T> New();
    //
    //    template <class T>
    //    Pointer<T> New(const T& val);
};

// template <class T>
// Pointer<T> Reflected::New()
//{
//     return Reg().New<T>();
// }
//
// template <class T>
// Pointer<T> Reflected::New(const T& val)
//{
//     return Reg().New(val);
// }

KAI_END

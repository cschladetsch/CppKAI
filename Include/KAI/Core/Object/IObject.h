#pragma once

#include <KAI/Core/Config/Base.h>
#include <KAI/Core/Type/Number.h>

#include "KAI/Core/Object/Label.h"

KAI_BEGIN

class IObject {
   public:
    enum Switch {
        Marked = 1,
        Managed = 2,
        Const = 4,
        Clean = 8,
        NoRecurse = 16,

        DefaultSwitches = Managed
    };

    enum Color { White, Grey, Black };
    virtual const StorageBase &GetStorageBase() const = 0;
    virtual StorageBase &GetStorageBase() = 0;

    virtual Type::Number GetTypeNumber() const = 0;
    virtual const ClassBase *GetClass() const = 0;

    virtual Registry *GetRegistry() const = 0;

    virtual Handle GetParentHandle() const = 0;
    virtual void SetParentHandle(Handle) = 0;
    virtual Handle GetHandle() const = 0;

    virtual bool Exists() const = 0;
    virtual bool Valid() const = 0;

    virtual bool IsConst() const = 0;
    virtual bool IsManaged() const = 0;
    virtual bool IsMarked() const = 0;
    virtual bool IsClean() const = 0;

    virtual void SetMarked(bool = true) const = 0;
    virtual void SetSwitch(int, bool) = 0;
    virtual void SetManaged(bool = true) = 0;
    virtual void SetClean(bool = true) = 0;

    /* apparently, none of these methods_ are used...
    virtual void SetSwitches(int) = 0;
    virtual void SetConst() = 0;
    virtual int GetSwitches() const = 0;
    void SetDirty() { SetClean(false); }
    bool IsMutable() const { return !IsConst(); }
    bool IsUnmanaged() const { return !IsManaged(); }
    bool IsUnmarked() const { return !IsMarked(); }
    */

    bool IsDirty() const { return !IsClean(); }

    virtual void Set(const Label &, const Object &) = 0;
    virtual Object Get(const Label &) = 0;
    virtual void Remove(const Label &) = 0;
    virtual bool Has(const Label &) = 0;

    virtual String ToString() const = 0;
    virtual String ToXmlString() const = 0;
};

KAI_END

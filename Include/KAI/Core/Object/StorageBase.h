#pragma once

#include <KAI/Core/ObjectColor.h>

#include <list>

#include "KAI/Core/BuiltinTypes/Dictionary.h"
#include "KAI/Core/Object/Object.h"

KAI_BEGIN

// Base for all object instances_. The value stored with an object is placed
// contiguously in memory with the object.
struct StorageBase : public Object {
   public:
    typedef int Switches;
    typedef std::list<Handle> Containers;

   private:
    Containers containers;
    Dictionary dictionary;
    Handle parent;
    Switches switches;
    Label label;
    ObjectColor::Color color;

   public:
    StorageBase(const ObjectConstructParams &P)
        : Object(P), switches(DefaultSwitches) {}

    virtual ~StorageBase() {}

    void Delete();

    bool SetColor(ObjectColor::Color C);
    ObjectColor::Color GetColor() const { return color; }
    void SetWhite() { SetColor(ObjectColor::White); }
    void SetGrey() { SetColor(ObjectColor::Grey); }
    void SetBlack() { SetColor(ObjectColor::Black); }
    bool IsWhite() const { return color == ObjectColor::White; }
    bool IsGrey() const { return color == ObjectColor::Grey; }
    bool IsBlack() const { return color == ObjectColor::Black; }

    const Label &GetLabel() const { return label; }
    void SetLabel(const Label &L) { label = L; }

    const Dictionary &GetDictionary() const { return dictionary; }
    Dictionary &GetDictionary() { return dictionary; }
    Object Get(const Label &) const;
    void Set(const Label &, Object const &);
    void Remove(const Label &);
    void Detach(const Label &L) { Remove(L); }
    void Detach(Object const &);
    bool Has(const Label &) const;

    void SetParentHandle(Handle H);
    Handle GetParentHandle() const { return parent; }

    void SetSwitch(int, bool);
    void SetSwitches(int S) { switches = S; }
    void SetMarked(bool B) { SetSwitch(Marked, B); }
    void SetManaged(bool B);
    void SetConstant(bool B) { SetSwitch(Const, B); }
    void SetClean(bool B = true);
    void SetDirty(bool B = true) { SetClean(!B); }
    int GetSwitches() const { return switches; }

    bool IsSwitchOn(Switch S) const { return (switches & S) != 0; }
    bool IsMarked() const { return IsSwitchOn(Marked); }
    bool IsManaged() const { return IsSwitchOn(Managed); }
    bool IsConst() const { return IsSwitchOn(Const); }
    bool IsClean() const { return IsSwitchOn(Clean); }

    Object &operator[](Label const &);
    Object const &operator[](Label const &) const;

    // private:
    void MakeReachableGrey();
    bool CanBlacken();
    void SetColorRecursive(ObjectColor::Color color);

    Containers const &GetContainers() const { return containers; }
    void RemovedFromContainer(Object const &container);
    void DetermineNewColor();
    void AddedToContainer(Object const &container);
    void DetachFromContainers();
    // protected:

    StorageBase *GetParentPtr();

    void SetColorRecursive(ObjectColor::Color color, HandleSet &handles);
};

KAI_END

// EOF

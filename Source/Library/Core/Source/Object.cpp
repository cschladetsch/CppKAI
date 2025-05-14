#include <iostream>

#include "KAI/Core/BuiltinTypes.h"

KAI_BEGIN

void ToStringStream(const Object &Q, StringStream &S, int level);
void ToXmlStream(const Object &Q, StringStream &S, int level);

ObjectConstructParams::ObjectConstructParams(Registry *R, const ClassBase *C,
                                             Handle H, Constness K)
    : registry(R), class_base(C), handle(H), constness(K) {}

ObjectConstructParams::ObjectConstructParams(StorageBase *Q) {
    registry = Q->GetRegistry();
    handle = Q->GetHandle();
    class_base = Q->GetClass();
}

Object::Object(const ObjectConstructParams &P)
    : registry(P.registry), class_base(P.class_base), handle(P.handle) {}

Object::Object(Object const &Q) { *this = Q; }

void Object::Assign(StorageBase &A, StorageBase const &B) {
    GetClass()->Assign(A, B);
}

Object &Object::operator=(Object const &Q) {
    // if we are assigning over an existing object, catch any leak by
    // determining new color for the assignee
    if (Valid()) {
        StorageBase *base = GetRegistry()->GetStorageBase(handle);
        if (base) base->DetermineNewColor();
    }

    class_base = Q.class_base;
    registry = Q.registry;
    handle = Q.handle;

    return *this;
}

void Object::Delete() const {
    if (!Valid()) return;

    StorageBase *base = GetBasePtr();
    if (base == 0) return;

    if (!base->IsManaged()) base->SetManaged(true);

    base->Delete();
}

Handle Object::GetParentHandle() const {
    return GetStorageBase().GetParentHandle();
}

void Object::SetParentHandle(Handle H) { GetStorageBase().SetParentHandle(H); }

bool Object::IsConst() const {
    if (!Exists()) return true;

    return GetStorageBase().IsConst();
}

int Object::GetSwitches() const {
    if (!Exists()) return 0;

    return GetStorageBase().GetSwitches();
}

bool Object::IsManaged() const {
    if (!Exists()) return true;

    return GetStorageBase().IsManaged();
}

bool Object::IsMarked() const {
    if (!Exists()) return false;

    return GetStorageBase().IsMarked();
}

bool Object::IsClean() const {
    if (!Exists()) return true;

    return GetStorageBase().IsClean();
}

void Object::SetSwitches(int S) const {
    if (Exists()) GetStorageBase().SetSwitches(S);
}

void Object::SetMarked(bool B) const {
    MarkObject(const_cast<Object &>(*this), B);
}

void Object::SetConst() const {
    if (Exists()) GetStorageBase().SetConst();
}

void Object::SetManaged(bool B) const {
    if (Exists()) GetStorageBase().SetManaged(B);
}

void Object::SetClean(bool B) const { GetStorageBase().SetClean(B); }

String Object::ToString() const {
    StringStream S;
    ToStringStream(*this, S, 0);
    return S.ToString();
}

String Object::ToXmlString() const {
    StringStream S;
    S << "\n";
    ToXmlStream(*this, S, 0);
    return S.ToString();
}

Type::Number Object::GetTypeNumber() const {
    if (!Valid()) return Type::Number::None;

    return GetClass()->GetTypeNumber();
}

bool Object::Valid() const {
    if (registry == 0) return false;

    if (handle.GetValue() == 0) return false;

    if (class_base == 0) return false;

    return true;
}

bool Object::Exists() const {
    return Valid() && registry->GetStorageBase(handle) != 0;
}

bool Object::OnDeathRow() const { return registry->OnDeathRow(handle); }

StorageBase *Object::GetStorageBase(Handle handle) const {
    return registry->GetStorageBase(handle);
}

StorageBase &Object::GetStorageBase() const {
    if (!Valid()) KAI_THROW_0(NullObject);

    return KAI_NAMESPACE(GetStorageBase(*this));
}

Object GetParent(const Object &Q) {
    if (!Q.Valid()) return Object();

    Handle handle = Q.GetParentHandle();
    if (handle == Handle()) return Object();

    return Q.GetRegistry()->GetObject(handle);
}

Object Object::GetParent() const { return KAI_NAMESPACE(GetParent)(*this); }

Type::Number GetTypeNumber(Object const &Q) {
    if (Q.GetHandle() == Handle(0)) return Type::Number::None;

    const ClassBase *klass = Q.GetClass();
    if (klass == 0) return Type::Number::None;

    return klass->GetTypeNumber();
}

StorageBase &GetStorageBase_(Object const &Q) {
    StorageBase *base = Q.GetRegistry()->GetStorageBase(Q.GetHandle());
    if (base == 0) KAI_THROW_0(NullObject);

    return *base;
}

StorageBase &GetStorageBase(Object const &Q) {
    if (!Q.Valid()) KAI_THROW_0(NullObject);

    return GetStorageBase_(Q);
}

StringStream &operator>>(StringStream &S, Object &Q) {
    if (Q.GetRegistry() == 0) KAI_THROW_1(Base, "NullRegistry");

    Q.GetClass()->ExtractValue(Q, S);

    return S;
}

void Object::Set(const Label &L, const Object &Q) const {
    GetStorageBase().StorageBase::Set(L, Q);
}

Object Object::Get(const Label &L) const { return GetStorageBase().Get(L); }

bool Object::Has(const Label &L) const { return GetStorageBase().Has(L); }

void Object::Remove(const Label &L) const { GetStorageBase().Remove(L); }

void Object::Detach(const Object &Q) const { GetStorageBase().Detach(Q); }

void Object::SetSwitch(int S, bool M) const {
    if (!Exists()) return;

    StorageBase &base = GetStorageBase();
    GetClass()->SetSwitch(base, S, M);
}

Dictionary const &Object::GetDictionary() const {
    if (!Exists()) KAI_THROW_0(NullObject);

    return GetStorageBase().GetDictionary();
}

Dictionary &Object::GetDictionaryRef() {
    if (!Exists()) KAI_THROW_0(NullObject);

    return GetStorageBase().GetDictionary();
}

void Object::Register(Registry &R) {
    KAI_UNUSED_1(R);
    ClassBuilder<Object>(R, "Object");
}

void Object::SetPropertyValue(Label const &L, Object const &V) const {
    KAI_NAMESPACE(SetPropertyValue(*this, L, V));
}

Object Object::GetPropertyValue(Label const &L) const {
    return KAI_NAMESPACE(GetPropertyValue(*this, L));
}

void Object::SetPropertyObject(Label const &L, Object const &V) const {
    KAI_NAMESPACE(SetPropertyObject(*this, L, V));
}

bool Object::HasProperty(Label const &name) const {
    return GetClass()->HasProperty(name);
}

Object Object::GetPropertyObject(Label const &L) const {
    return KAI_NAMESPACE(GetPropertyObject)(*this, L);
}

Label Object::GetLabel() const { return GetStorageBase().GetLabel(); }

Object Object::Duplicate() const { return KAI_NAMESPACE(Duplicate)(*this); }

Object Duplicate(Object const &Q) {
    if (!Q.Exists()) return Object();

    return Q.GetClass()->Duplicate(Q.GetStorageBase());
}

void Object::SetColor(ObjectColor::Color C) const {
    if (Exists()) GetStorageBase().SetColor(C);
}

void Object::SetColorRecursive(ObjectColor::Color C) const {
    if (Exists()) {
        StorageBase &base = GetStorageBase();
        base.SetColorRecursive(C);
    }
}

void Object::SetColorRecursive(ObjectColor::Color C, HandleSet &H) const {
    if (Exists()) GetStorageBase().SetColorRecursive(C, H);
}

ObjectColor::Color Object::GetColor() const {
    return GetStorageBase().GetColor();
}

void Object::RemovedFromContainer(Object Q) const {
    GetStorageBase().RemovedFromContainer(Q);
}

void Object::AddedToContainer(Object Q) const {
    GetStorageBase().AddedToContainer(Q);
}

StorageBase *Object::GetBasePtr() const {
    if (!Valid()) return 0;

    return GetRegistry()->GetStorageBase(GetHandle());
}

StorageBase *Object::GetParentBasePtr() const {
    if (!Valid()) return 0;

    StorageBase *base = GetBasePtr();
    if (!base) return 0;

    return GetRegistry()->GetStorageBase(base->GetParentHandle());
}

void Object::GetPropertyObjects(ObjectList &contained) const {
    if (!Exists()) return;

    GetClass()->GetPropertyObjects(GetStorageBase(), contained);
}

void Object::GetContainedObjects(ObjectList &contained) const {
    if (!Exists()) return;

    GetClass()->GetContainedObjects(GetStorageBase(), contained);
}

void Object::GetChildObjects(ObjectList &contained) const {
    if (!Exists()) return;

    for (auto &child : GetDictionary()) contained.push_back(child.second);
}

void Object::GetAllReferencedObjects(ObjectList &contained) const {
    GetPropertyObjects(contained);
    GetContainedObjects(contained);
    GetChildObjects(contained);
}

HashValue GetHash(Object const &Q) {
    if (!Q.Valid()) return Type::Number::None;

    ClassBase const *K = Q.GetClass();
    if (!K->HasTraitsProperty(Type::Properties::NoHashValue))
        return K->GetHashValue(Q.GetStorageBase());

    return 13;
}

BinaryStream &operator<<(BinaryStream &stream, const Object &object) {
    if (!object.Exists()) return stream << 0;

    const StorageBase &base = GetStorageBase(object);
    stream << base.GetTypeNumber().ToInt();

    // insert the value
    ClassBase const &klass = *base.GetClass();
    if (klass.HasOperation(Type::Properties::BinaryStreamInsert))
        klass.Insert(stream, base);

    // insert any properties_
    for (auto const &prop_iter : klass.GetProperties()) {
        stream << prop_iter.second->GetValue(object);
    }

    // insert sub-objects
    const Dictionary &dict = base.GetDictionary();
    stream << (int)dict.size();
    for (auto const &child : dict) stream << child.first << child.second;

    return stream;
}

BinaryStream &operator>>(BinaryStream &stream, Object &extracted) {
    if (stream.GetRegistry() == 0) KAI_THROW_1(Base, "NullRegistry");

    Registry &registry = *stream.GetRegistry();
    int type_number = 0;
    stream >> type_number;
    if (type_number == 0) {
        extracted = Object();
        return stream;
    }

    ClassBase const *klass = registry.GetClass(Type::Number(type_number));
    if (klass == 0) KAI_THROW_1(UnknownClass<>, type_number);

    // extract the object value
    if (klass->HasOperation(Type::Properties::BinaryStreamExtract))
        extracted = *klass->Extract(registry, stream);
    else
        extracted = registry.NewFromTypeNumber(type_number);

    // extract any properties_
    for (ClassBase::Properties::value_type const &prop_iter :
         klass->GetProperties()) {
        Object prop_value;
        stream >> prop_value;
        prop_iter.second->SetValue(extracted, prop_value);
    }

    // extract sub-objects
    int num_children = 0;
    stream >> num_children;
    for (int N = 0; N < num_children; ++N) {
        Label label;
        Object child;
        stream >> label >> child;
        extracted.Set(label, child);
    }

    return stream;
}

Object::ChildProxy::ChildProxy(Object const &Q, const char *P)
    : registry(Q.GetRegistry()), handle(Q.GetHandle()), label(P) {}

Object::ChildProxy::ChildProxy(Object const &Q, Label const &L)
    : registry(Q.GetRegistry()), handle(Q.GetHandle()), label(L) {}

Object Object::ChildProxy::GetObject() const {
    if (!registry) KAI_THROW_1(UnknownObject, Handle(0));

    StorageBase *base = registry->GetStorageBase(handle);
    if (!base) KAI_THROW_1(UnknownObject, handle);

    return *base;
}

Object Absolute(Object const &A) {
    if (!A.Exists()) return A;

    return A.GetClass()->Absolute(A);
}

// This is a vitally important operation that affects everything about
// rationalising about general programming.
//
// An object is less-than another if it doesn't exist and the other does,
// or if the first object's class compares them as less.
bool operator<(Object const &A, Object const &B) {
    if (!A.Exists()) return B.Exists();

    if (!B.Exists()) return false;

    // test value
    ClassBase const &klass_a = *A.GetClass();
    ClassBase const &klass_b = *B.GetClass();
    if (klass_a.HasOperation(Type::Properties::Less)) {
        if (klass_a.Less(A, B)) return true;

        if (klass_b.HasOperation(Type::Properties::Less) && klass_b.Less(B, A))
            return false;
    }

    // test properties_
    for (ClassBase::Properties::value_type const &prop :
         klass_a.GetProperties()) {
        Object prop_a = prop.second->GetValue(A);
        Object prop_b = B.Get(prop.second->GetFieldName());

        if (prop_a < prop_b) return true;

        if (prop_b < prop_a) return false;
    }

    // test sub-objects
    for (Dictionary::value_type const &child_a_entry : A.GetDictionary()) {
        Object child_a = child_a_entry.second;
        Object child_b = B.Get(child_a_entry.first);

        if (child_a < child_b) return true;

        if (child_b < child_a) return false;
    }

    // objects are seemingly equivalent
    return false;
}

/// Two objects are equivalent if they both do not exist, or if
/// the first object's class compares them as equivalent and all
/// sub-objects are equivalent.
bool operator==(Object const &A, Object const &B) {
    if (!A.Exists()) return !B.Exists();

    if (!B.Exists()) return false;

    // test value
    ClassBase const &klass = *A.GetClass();
    if (klass.HasOperation(Type::Properties::Equiv) && !klass.Equiv(A, B))
        return false;

    // compare dictionary sizes
    Dictionary const &dict_a = A.GetDictionary();
    Dictionary const &dict_b = B.GetDictionary();
    if (dict_a.size() != dict_b.size()) return false;

    // test properties_
    for (auto const &prop : klass.GetProperties()) {
        Object prop_a = prop.second->GetValue(A);
        Object prop_b = B.Get(prop.second->GetFieldName());
        if (!(prop_a == prop_b)) return false;
    }

    // test sub-objects
    for (auto const &child_a_entry : dict_a) {
        Object child_a = child_a_entry.second;
        Object child_b = B.Get(child_a_entry.first);
        if (!(child_a == child_b)) return false;
    }

    return true;
}

// Uses Less-than and equivalence testing, iff no greater-than is defined.
bool operator>(Object const &A, Object const &B) {
    if (!A.Exists()) return false;

    if (!B.Exists()) return A.Exists();

    if (A.GetClass()->HasOperation(Type::Properties::Greater))
        return A.GetClass()->Greater(A, B);

    return A.GetClass()->Less(B, A) && !A.GetClass()->Equiv(A, B);
}

Object operator+(Object const &A, Object const &B) {
    if (!A.Exists() || !B.Exists()) KAI_THROW_0(NullObject);

    return A.GetClass()->Plus(A, B);
}

Object operator-(Object const &A, Object const &B) {
    if (!A.Exists() || !B.Exists()) KAI_THROW_0(NullObject);

    return A.GetClass()->Minus(A, B);
}

Object operator*(Object const &A, Object const &B) {
    if (!A.Exists() || !B.Exists()) KAI_THROW_0(NullObject);

    return A.GetClass()->Multiply(A, B);
}

Object operator/(Object const &A, Object const &B) {
    if (!A.Exists() || !B.Exists()) KAI_THROW_0(NullObject);

    return A.GetClass()->Divide(A, B);
}

Object Object::NewFromTypeNumber(Type::Number N) const {
    return registry->NewFromTypeNumber(N);
}

KAI_END

// EOF

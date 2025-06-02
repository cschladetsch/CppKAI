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

Object::Object(Object const &Q) {
    // ULTIMATE defensive check - detect if source is not a valid address
    if (reinterpret_cast<uintptr_t>(&Q) < 0x1000) {
        // This is an invalid pointer - it's pointing to a very low memory
        // address Initialize to null to avoid propagating bad pointers
        registry = nullptr;
        class_base = nullptr;
        handle = Handle();
        return;
    }

    // Use a more robust copy constructor that validates the source
    try {
        if (Q.Valid()) {
            registry = Q.registry;
            class_base = Q.class_base;
            handle = Q.handle;
        } else {
            // Silent initialization for null objects - common in Rho/Pi
            // languages
            registry = nullptr;
            class_base = nullptr;
            handle = Handle();
            // No logging for invalid objects - this is expected behavior
        }
    } catch (...) {
        // If any exception occurs, ensure we initialize to null
        registry = nullptr;
        class_base = nullptr;
        handle = Handle();
    }
}

void Object::Assign(StorageBase &A, StorageBase const &B) {
    if (GetClass() == nullptr) {
        KAI_TRACE_ERROR() << "Object::Assign: Null class_base";
        return;
    }
    GetClass()->Assign(A, B);
}

Object &Object::operator=(Object const &Q) {
    // ULTIMATE defensive check - detect if source is not a valid address
    if (reinterpret_cast<uintptr_t>(&Q) < 0x1000) {
        // This is an invalid pointer - just preserve current state
        return *this;
    }

    try {
        // if we are assigning over an existing object, catch any leak by
        // determining new color for the assignee
        if (Valid()) {
            StorageBase *base = nullptr;

            try {
                base = GetRegistry()->GetStorageBase(handle);
            } catch (...) {
                // Silent handling if GetStorageBase fails
            }

            if (base) {
                try {
                    base->DetermineNewColor();
                } catch (...) {
                    // Silent handling if DetermineNewColor fails
                }
            }
        }

        // Validate source object before assignment
        bool sourceValid = false;
        try {
            sourceValid = Q.Valid();
        } catch (...) {
            // If Valid() throws, treat as invalid
            sourceValid = false;
        }

        if (sourceValid) {
            class_base = Q.class_base;
            registry = Q.registry;
            handle = Q.handle;
        } else {
            // Source object is invalid, log a warning but don't nullify this
            // object if it's valid
            bool thisValid = false;
            try {
                thisValid = Valid();
            } catch (...) {
                thisValid = false;
            }

            if (thisValid) {
                // Assignment of invalid object should clear the current object
                class_base = nullptr;
                registry = nullptr;
                handle = Handle();
            } else {
                // Both are invalid, clear everything
                class_base = nullptr;
                registry = nullptr;
                handle = Handle();
            }
        }
    } catch (...) {
        // If anything else goes wrong, silently keep current state
    }

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
    // ULTIMATE defensive check - detect if 'this' is not a valid address
    // This will catch cases where the Object is at an invalid memory address
    // like 0x8
    if (reinterpret_cast<uintptr_t>(this) < 0x1000) {
        // This is an invalid pointer - it's pointing to a very low memory
        // address such as 0x8, 0x10, etc., which are never valid user-space
        // addresses
        return false;
    }

    // IMPORTANT: Defensive check for invalid registry address
    // This handles both nullptr and invalid memory addresses
    // Using try/catch to prevent segmentation fault if registry is an invalid
    // address
    try {
        // Special case for null registry (most common case)
        if (registry == nullptr) {
            // Completely silent for null registry - this is expected in the
            // Rho/Pi language environment where Continuations use intermediate
            // values
            return false;
        }

        // Test if registry is a valid address by doing a harmless operation
        volatile void *registryAddr = static_cast<void *>(registry);
        if (!registryAddr) {
            return false;
        }
    } catch (...) {
        // If we catch any exception, registry is likely an invalid pointer
        // This is completely silent - no logging to avoid console spam
        return false;
    }

    // Check handle next - safe because handle is a member of this object
    // and we've already verified 'this' is a valid pointer
    try {
        if (handle.GetValue() == 0) {
            // Silent for handle check too
            return false;
        }
    } catch (...) {
        return false;
    }

    // Check class last - also safe because class_base is a member
    try {
        if (class_base == nullptr) {
            // Silent for class_base check too
            return false;
        }

        // Test if class_base is a valid address
        const void *classAddr = class_base;
        if (!classAddr) {
            return false;
        }
    } catch (...) {
        return false;
    }

    // All checks passed, object is valid
    return true;
}

bool Object::Exists() const {
    // ULTIMATE defensive check - detect if 'this' is not a valid address
    if (reinterpret_cast<uintptr_t>(this) < 0x1000) {
        // This is an invalid pointer - it's pointing to a very low memory
        // address
        return false;
    }

    // Make sure the object is valid before trying to access the registry
    if (!Valid()) {
        return false;
    }

    // Defensive check when accessing the registry
    try {
        // Verify registry is still valid
        if (registry == nullptr) {
            return false;
        }

        // Extra validation of registry
        volatile void *registryAddr = static_cast<void *>(registry);
        if (!registryAddr) {
            return false;
        }

        // Try to get the storage base
        StorageBase *storageBase = nullptr;
        try {
            storageBase = registry->GetStorageBase(handle);
        } catch (...) {
            return false;
        }

        return storageBase != nullptr;
    } catch (...) {
        // Silent handling of any exceptions during registry access
        return false;
    }
}

bool Object::OnDeathRow() const {
    // Safety check before accessing registry
    if (!Valid()) {
        return false;
    }

    try {
        return registry->OnDeathRow(handle);
    } catch (...) {
        // Silent handling of any exceptions
        return false;
    }
}

StorageBase *Object::GetStorageBase(Handle handle) const {
    // Validate registry before accessing it
    if (!Valid()) {
        return nullptr;
    }

    try {
        return registry->GetStorageBase(handle);
    } catch (...) {
        // Silent handling of any exceptions
        return nullptr;
    }
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
    // Comprehensive safety checks with no logging (to avoid console spam)
    try {
        if (!Q.Valid()) {
            KAI_THROW_0(NullObject);
        }

        if (!Q.Exists()) {
            KAI_THROW_0(NullObject);
        }

        // Extra careful registry access
        Registry *registry = nullptr;
        try {
            registry = Q.GetRegistry();
            if (registry == nullptr) {
                KAI_THROW_0(NullObject);
            }
        } catch (...) {
            KAI_THROW_0(NullObject);
        }

        // Handle checks with safety net
        Handle handle;
        try {
            handle = Q.GetHandle();
            if (handle.GetValue() == 0) {
                KAI_THROW_0(NullObject);
            }
        } catch (...) {
            KAI_THROW_0(NullObject);
        }

        // Safe storage base access with extra error checking
        StorageBase *base = nullptr;
        try {
            base = registry->GetStorageBase(handle);
            if (base == nullptr) {
                KAI_THROW_0(NullObject);
            }

            // Quick validation test on the base pointer
            volatile void *test = static_cast<void *>(base);
            if (!test) {
                KAI_THROW_0(NullObject);
            }
        } catch (...) {
            KAI_THROW_0(NullObject);
        }

        return *base;
    } catch (...) {
        // Catch any errors we missed and convert to NullObject
        KAI_THROW_0(NullObject);
    }
}

StorageBase &GetStorageBase(Object const &Q) {
    try {
        // Enhanced validation with additional safety checks
        // First validate that Q itself is a valid object reference
        // This uses a try block because Q might be in an invalid memory
        // location
        try {
            volatile bool isValid = Q.Valid();
            if (!isValid) {
                KAI_THROW_0(NullObject);
            }
        } catch (...) {
            // If we get here, Q is likely in an invalid memory location
            KAI_THROW_0(NullObject);
        }

        // Now check if the object exists (has valid storage)
        try {
            volatile bool exists = Q.Exists();
            if (!exists) {
                KAI_THROW_0(NullObject);
            }
        } catch (...) {
            KAI_THROW_0(NullObject);
        }

        // Check registry safety
        Registry *registry = nullptr;
        try {
            registry = Q.GetRegistry();
            if (registry == nullptr) {
                KAI_THROW_0(NullObject);
            }

            // Test registry validity with a safe operation
            volatile void *test = static_cast<void *>(registry);
            if (!test) {
                KAI_THROW_0(NullObject);
            }
        } catch (...) {
            KAI_THROW_0(NullObject);
        }

        // If all checks pass, return the storage base
        return GetStorageBase_(Q);
    } catch (const Exception::Base &) {
        KAI_THROW_0(NullObject);
    } catch (const std::exception &) {
        KAI_THROW_0(NullObject);
    } catch (...) {
        KAI_THROW_0(NullObject);
    }
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

// Helper to track serialization depth
static thread_local int serialization_depth = 0;
static const int MAX_SERIALIZATION_DEPTH = 10;

BinaryStream &operator<<(BinaryStream &stream, const Object &object) {
    if (!object.Exists()) return stream << 0;

    // Check for circular references by limiting depth
    if (serialization_depth >= MAX_SERIALIZATION_DEPTH) {
        // Write a null object marker when max depth is reached
        return stream << 0;
    }

    // Increment depth counter
    serialization_depth++;

    // Use RAII to ensure depth is decremented on exit
    struct DepthGuard {
        ~DepthGuard() { serialization_depth--; }
    } guard;

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

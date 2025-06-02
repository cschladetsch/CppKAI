#include <KAI/Core/BuiltinTypes/Array.h>
#include <KAI/Core/Object/IObject.h>
#include <KAI/Core/Registry.h>

#include <algorithm>
#include <map>

KAI_BEGIN

void StorageBase::Detach(Object const &parent) {
    // The object calling Detach wants to be removed from the given parent
    StorageBase *parentBase = GetRegistry()->GetStorageBase(parent.GetHandle());
    if (!parentBase) return;

    // First check if parent has this object in its dictionary
    for (auto const &[label, object] : parentBase->dictionary) {
        if (object.GetHandle() == GetHandle()) {
            parentBase->Remove(label);
            return;
        }
    }

    // For containers like Array, we need to call their Erase method directly
    // since the generic ContainerOps::Erase is not implemented
    if (parentBase->GetClass()->GetTypeNumber() == Type::Number::Array) {
        // Cast to Array and call Erase
        Array &array = Deref<Array>(Object(parentBase));
        array.Erase(GetHandle());
    } else {
        // Otherwise, ask parent to detach this object from its container
        parentBase->GetClass()->DetachFromContainer(*parentBase, *this);
    }
}

void StorageBase::SetSwitch(int S, bool N) {
    if (N)
        switches |= S;
    else
        switches &= ~S;

    const ClassBase *klass = GetClass();
    if (klass != nullptr && S != Clean) klass->SetSwitch(*this, S, N);
}

void StorageBase::SetParentHandle(Handle H) { parent = H; }

Object StorageBase::Get(const Label &L) const {
    if (GetClass()->HasProperty(L))
        return GetClass()->GetProperty(L).GetValue(*this);

    Dictionary::const_iterator A = dictionary.find(L);
    if (A == dictionary.end()) return Object();

    return A->second;
}

void StorageBase::Set(const Label &name, Object const &child) {
    if (child.GetHandle() == GetHandle())
        KAI_THROW_1(InternalError, "Recursion");

    // Mark the object as being altered.
    SetDirty();

    // Set a property if it exists.
    ClassBase const *klass = GetClass();
    if (klass->HasProperty(name)) {
        klass->GetProperty(name).SetValue(*this, child);
        return;
    }

    // otherwise this is a child object. remove any existing child
    Remove(name);

    // update the child object
    if (!child.Exists()) {
        const auto ch = dictionary.find(name);
        if (ch != dictionary.end()) dictionary.erase(ch);

        return;
    }

    StorageBase &base = KAI_NAMESPACE(GetStorageBase(child));
    base.SetLabel(name);
    base.SetParentHandle(GetHandle());

    bool clean = base.IsClean();
    bool konst = base.IsConst();
    bool managed = base.IsManaged();
    base.switches = switches;  // inherit properties_ of parent...

    if (clean)  // ...but preserve cleanliness
        base.switches |= IObject::Clean;
    else
        base.switches &= ~IObject::Clean;

    if (konst)  // ...and constness
        base.switches |= IObject::Const;

    if (managed)  // ...and managed
        base.switches |= IObject::Managed;

    // Add it to this dictionary, inform it of being added to a container.
    dictionary[name] = child;
    base.AddedToContainer(*this);
}

bool StorageBase::Has(const Label &L) const {
    const auto object = dictionary.find(L);
    return object != dictionary.end() && object->second.Exists();
}

void StorageBase::Remove(const Label &label) {
    const auto found = dictionary.find(label);
    if (found == dictionary.end()) return;

    SetDirty();
    StorageBase *child = found->second.GetBasePtr();
    dictionary.erase(found);

    if (child) {
        child->SetParentHandle(Handle());
        child->RemovedFromContainer(*this);
    }
}

void StorageBase::SetColorRecursive(ObjectColor::Color color) {
    HandleSet handles;
    SetColorRecursive(color, handles);
}

// avoid loops by passing history of objects traversed via handles argument
void StorageBase::SetColorRecursive(ObjectColor::Color color,
                                    HandleSet &handles) {
    Handle handle = GetHandle();
    if (handles.find(handle) != handles.end()) return;

    handles.insert(handle);

    if (!SetColor(color)) return;

    GetClass()->SetReferencedObjectsColor(*this, color, handles);
    if (dictionary.empty()) return;

    // Use non-recursive iteration with a stack to avoid stack overflow
    std::vector<StorageBase *> stack;

    // First pass: add all direct children to the stack
    for (Dictionary::value_type const &child : dictionary) {
        StorageBase *sub =
            GetRegistry()->GetStorageBase(child.second.GetHandle());
        if (sub && handles.find(sub->GetHandle()) == handles.end()) {
            stack.push_back(sub);
            handles.insert(sub->GetHandle());
        }
    }

    // Process the stack iteratively
    while (!stack.empty()) {
        StorageBase *current = stack.back();
        stack.pop_back();

        if (!current->SetColor(color)) continue;

        current->GetClass()->SetReferencedObjectsColor(*current, color,
                                                       handles);

        // Add all child objects to the stack if not already processed
        for (Dictionary::value_type const &child : current->dictionary) {
            StorageBase *sub =
                GetRegistry()->GetStorageBase(child.second.GetHandle());
            if (sub && handles.find(sub->GetHandle()) == handles.end()) {
                stack.push_back(sub);
                handles.insert(sub->GetHandle());
            }
        }
    }
}

bool StorageBase::SetColor(ObjectColor::Color color) {
    auto reg = GetRegistry();
    if (!reg->SetColor(*this, color)) return false;

    this->color = color;
    if (color == ObjectColor::White) {
        for (const auto &container : containers) {
            StorageBase *cont = GetRegistry()->GetStorageBase(container);
            if (cont && cont->IsBlack()) cont->SetColor(ObjectColor::Grey);
        }
    }

    return true;
}

void StorageBase::MakeReachableGrey() {
    for (const auto &child : dictionary) {
        StorageBase *sub =
            GetRegistry()->GetStorageBase(child.second.GetHandle());
        if (!sub) continue;

        if (sub->IsWhite()) sub->SetColor(ObjectColor::Grey);
    }

    GetClass()->MakeReachableGrey(*this);
}

bool StorageBase::CanBlacken() {
    // Check if all children can be blackened
    for (const auto &[_, child] : dictionary) {
        StorageBase *sub = GetRegistry()->GetStorageBase(child.GetHandle());
        if (!sub || sub->IsWhite()) {
            return false;
        }
    }

    // Check if referenced objects can be blackened
    return GetClass()->CanBlackenReferencedObjects(*this);
}

void StorageBase::RemovedFromContainer(Object const &container) {
    ObjectColor::Color color = ObjectColor::White;
    StorageBase *parent = GetRegistry()->GetStorageBase(GetParentHandle());
    bool parent_is_black = parent && parent->IsBlack();
    if (parent_is_black) color = ObjectColor::Grey;

    bool removed = false;
    auto iter = containers.begin(), end = containers.end();
    for (; iter != end;) {
        StorageBase *base = GetRegistry()->GetStorageBase(*iter);
        if (!base) {
            iter = containers.erase(iter);
            continue;
        }

        if (!removed && *iter == container.GetHandle()) {
            iter = containers.erase(iter);
            removed = true;
            if (parent_is_black) {
                // if removed from container and parent is black_ early out
                break;
            } else {
                // we need to check for other black_ parents to enforce the
                // TriColor invariant
                continue;
            }
        }

        if (base->IsBlack()) {
            color = ObjectColor::Grey;
            parent_is_black = true;
            // if any parent container is black_, and we have already removed
            // from the given container, we can early out
            if (removed) break;
        }

        ++iter;
    }

    SetColorRecursive(color);
}

void StorageBase::DetermineNewColor() {
    // removing from an empty container will still traverse through other
    // containers to determine new color
    RemovedFromContainer(Object());
}

void StorageBase::AddedToContainer(Object const &container) {
    if (container.GetHandle() == GetHandle())
        KAI_THROW_1(InternalError, "Can't add a container to itself.");

    containers.push_back(container.GetHandle());
    if (IsWhite()) SetGrey();
}

void StorageBase::SetClean(bool clean) {
    SetSwitch(Clean, clean);
    if (!clean && IsBlack()) SetColor(ObjectColor::Grey);
}

void StorageBase::DetachFromContainers() {
    if (containers.empty()) return;

    Containers tmp = containers;
    Containers::const_iterator iter = tmp.begin(), end = tmp.end();
    for (; iter != end; ++iter) {
        StorageBase *cont = GetRegistry()->GetStorageBase(*iter);
        if (!cont) continue;

        cont->GetClass()->DetachFromContainer(*cont, *this);
    }
}

void StorageBase::Delete() {
    // Avoid double deletion.
    if (IsMarked()) return;

    SetManaged(true);

    // Remove from all containers.
    DetachFromContainers();

    // remove from parent
    StorageBase *parent = GetParentBasePtr();
    if (parent != 0) parent->Remove(GetLabel());

    // Set this and all referent objects to be white_, and mark it for deletion.
    SetColorRecursive(ObjectColor::White);
    SetMarked(true);
}

void StorageBase::SetManaged(bool managed) {
    if (!managed) SetColor(ObjectColor::Black);

    SetSwitch(Managed, managed);
}

KAI_END

// EOF

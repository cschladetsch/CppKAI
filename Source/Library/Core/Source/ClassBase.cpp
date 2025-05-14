#include <KAI/Core/Object/Class.h>

KAI_BEGIN

void RegisterClass(Registry &reg, ClassBase const &klass, Object const &root,
                   Pathname const &path) {
    // TODO: what were they ever for?
    KAI_UNUSED_2(root, path);
    reg.AddClass(&klass);
}

ClassBase::~ClassBase() {
    for (const auto &[_, snd] : methods_) delete snd;

    for (const auto &[_, snd] : properties_) delete snd;
}

void ClassBase::SetReferencedObjectsColor(StorageBase &base,
                                          ObjectColor::Color color,
                                          HandleSet &handles) const {
    if (properties_.empty()) return;

    for (const auto &[_, propertyBase] : properties_) {
        PropertyBase const &prop = *propertyBase;
        if (!prop.IsSystemType()) continue;

        auto property = prop.GetObject(base);

        if (!property.Exists()) continue;

        property.SetColorRecursive(color, handles);
    }
}

void ClassBase::SetMarked(StorageBase &Q, bool M) const {
    for (auto const &[_, property] : properties_) property->SetMarked(Q, M);

    SetMarked2(Q, M);
}

void ClassBase::MakeReachableGrey(StorageBase &base) const {
    if (properties_.empty()) return;
    for (auto const &[_, prop] : properties_) {
        if (!prop->IsSystemType()) continue;

        auto property = prop->GetObject(base);
        auto storageBase =
            property.GetRegistry()->GetStorageBase(property.GetHandle());

        if (storageBase == nullptr) continue;

        if (storageBase->IsWhite()) storageBase->SetColor(ObjectColor::Grey);
    }
}

void ClassBase::GetPropertyObjects(StorageBase &object,
                                   ObjectList &contained) const {
    if (properties_.empty()) return;

    for (auto const &[_, prop] : properties_) {
        if (!prop->IsSystemType()) continue;

        Object property = prop->GetObject(object);
        StorageBase *base =
            property.GetRegistry()->GetStorageBase(property.GetHandle());

        if (base == 0) continue;

        contained.push_back(*base);
    }
}

KAI_END

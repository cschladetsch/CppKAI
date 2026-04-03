#include <KAI/Core/Config/Base.h>
#include <KAI/Core/Object/ClassBase.h>
#include <KAI/Core/Object/Object.h>
#include <KAI/Core/Object/PropertyBase.h>

KAI_BEGIN

void SetPropertyValue(Object const &owner, Label const &name, Object value) {
    if (!owner.Valid()) KAI_THROW_0(NullObject);

    ClassBase const *klass = owner.GetClass();
    PropertyBase const &prop = klass->GetProperty(name);
    prop.SetValue(owner, value);
}

Object GetPropertyValue(Object const &owner, Label const &name) {
    if (!owner.Exists()) KAI_THROW_0(NullObject);

    ClassBase const *klass = owner.GetClass();
    PropertyBase const &prop = klass->GetProperty(name);

    return prop.GetValue(owner);
}

void SetPropertyObject(Object const &owner, Label const &name, Object value) {
    if (!owner.Exists()) KAI_THROW_0(NullObject);

    ClassBase const *klass = owner.GetClass();
    PropertyBase const &prop = klass->GetProperty(name);
    prop.SetObject(owner, value);
}

Object GetPropertyObject(Object const &owner, Label const &name) {
    if (!owner.Exists()) KAI_THROW_0(NullObject);

    ClassBase const *klass = owner.GetClass();
    PropertyBase const &prop = klass->GetProperty(name);

    return prop.GetObject(owner);
}

KAI_END

// EOF

#pragma once

#include <KAI/Core/Type/Number.h>

#include "KAI/Core/BasePointer.h"
#include "KAI/Core/Object/Label.h"
#include "KAI/Core/Object/MemberCreateParams.h"
#include "KAI/UnfuckWin32.h"

KAI_BEGIN

/// nominates a field in an instance of a class. commonality for all accessors
/// and mutators.
class PropertyBase {
    Type::Number class_type;
    Type::Number field_type;
    Label field_name;

    /// True if this field is an Object or a Pointer<T> or convertable to either
    bool is_system;

    int create_params;

   public:
    PropertyBase(Label const &F, Type::Number C, Type::Number N, bool B,
                 MemberCreateParams::Enum CP)
        : field_name(F),
          class_type(C),
          field_type(N),
          is_system(B),
          create_params(CP) {}

    virtual ~PropertyBase() {}

    String Description;

    /// true if field is an Object or Pointer<T>
    bool IsSystemType() const { return is_system; }

    /// if true, this property should be created when its containing object is
    /// created
    bool CreateDefaultValue() const {
        return (create_params & MemberCreateParams::Create) ==
               MemberCreateParams::Create;
    }

    virtual void SetMarked(Object &, bool) = 0;

    virtual Object GetValue(Object const &) const = 0;
    virtual void SetValue(Object const &, Object const &) const = 0;

    virtual void SetObject(Object const &Q, Object const &V) const = 0;
    virtual Object GetObject(Object const &Q) const = 0;

    Label const &GetFieldName() const { return field_name; }
    Type::Number GetClassTypeNumber() const { return class_type; }
    Type::Number GetFieldTypeNumber() const { return field_type; }
};

StringStream &operator<<(StringStream &S, BasePointer<PropertyBase> const &);

KAI_TYPE_TRAITS(BasePointer<PropertyBase>, Number::Property,
                Properties::StringStreamInsert);

// access the property object
void SetPropertyObject(Object const &owner, Label const &name, Object value);
Object GetPropertyObject(Object const &owner, Label const &name);

// access the property value
void SetPropertyValue(Object const &owner, Label const &name, Object value);
Object GetPropertyValue(Object const &owner, Label const &name);

KAI_END

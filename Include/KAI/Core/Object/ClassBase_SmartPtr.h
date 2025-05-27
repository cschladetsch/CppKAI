#pragma once

#include <KAI/Core/Base.h>
#include <KAI/Core/Exception/Extended.h>
#include <KAI/Core/Type/Number.h>
#include <KAI/Core/Type/Properties.h>

#include <list>
#include <memory>
#include <unordered_map>

#include "KAI/Core/Exception/ExceptionMacros.h"
#include "KAI/Core/Object/Label.h"
#include "KAI/Core/Object/LabelHash.h"
#include "KAI/Core/Object/MethodBase.h"
#include "KAI/Core/Object/PropertyBase.h"
#include "KAI/Core/StringStream.h"

KAI_BEGIN

class MethodBase;
class PropertyBase;

/// Base for all Class<T> types with smart pointer management
class ClassBase {
   public:
    // UPDATED: Use unique_ptr for ownership of methods and properties
    typedef std::unordered_map<Label, std::unique_ptr<MethodBase>,
                               detail::LabelHash>
        Methods;
    typedef std::unordered_map<Label, std::unique_ptr<PropertyBase>,
                               detail::LabelHash>
        Properties;
    typedef std::list<Object> ObjectList;

   protected:
    Label name_;
    Methods methods_;
    Properties properties_;
    Type::Number type_number_;

   public:
    ClassBase(Label const &name, Type::Number T)
        : name_(name), type_number_(T) {}

    // Destructor is now trivial - unique_ptr handles cleanup
    virtual ~ClassBase() = default;

    const Label &GetName() const { return name_; }
    const Label &GetLabel() const { return GetName(); }
    Type::Number GetTypeNumber() const { return type_number_; }

    virtual void SetReferencedObjectsColor(StorageBase &base,
                                           ObjectColor::Color color,
                                           HandleSet &handles) const;

    void GetPropertyObjects(StorageBase &object, ObjectList &contained) const;

    /// Methods - now with smart pointer management
    void AddMethod(const Label &L, std::unique_ptr<MethodBase> M) {
        methods_[L] = std::move(M);
    }

    // Backward compatibility overload - takes ownership
    [[deprecated("Use AddMethod with unique_ptr")]]
    void AddMethod(const Label &L, MethodBase *M) {
        methods_[L] = std::unique_ptr<MethodBase>(M);
    }

    const Methods &GetMethods() const { return methods_; }

    MethodBase *GetMethod(const Label &L) const {
        const auto found = methods_.find(L);
        return found == methods_.end() ? nullptr : found->second.get();
    }

    /// Properties - now with smart pointer management
    void AddProperty(Label const &label,
                     std::unique_ptr<PropertyBase> property) {
        properties_[label] = std::move(property);
    }

    // Backward compatibility overload - takes ownership
    [[deprecated("Use AddProperty with unique_ptr")]]
    void AddProperty(Label const &label, PropertyBase *property) {
        properties_[label] = std::unique_ptr<PropertyBase>(property);
    }

    bool HasProperty(Label const &label) const {
        return properties_.find(label) != properties_.end();
    }

    // Note: This needs updating in calling code to handle unique_ptr
    const Properties &GetProperties() const { return properties_; }

    PropertyBase const &GetProperty(Label const &L) const {
        auto found = properties_.find(L);
        if (found == properties_.end())
            KAI_THROW_2(UnknownProperty, GetName(), L);
        return *found->second;
    }

    // Helper method to get property pointer (non-owning)
    PropertyBase *GetPropertyPtr(Label const &L) const {
        auto found = properties_.find(L);
        return found == properties_.end() ? nullptr : found->second.get();
    }

    bool HasOperation(int N) const { return HasTraitsProperty(N); }

    virtual void MakeReachableGrey(StorageBase &base) const = 0;
    virtual void CreateFunction(StorageBase &storage) const = 0;
    virtual void Delete(StorageBase &storage) const = 0;
    virtual bool HasTraitsProperty(int N) const = 0;
    virtual Type::Properties GetProperties() const = 0;
    virtual StorageBase *NewStorage(Registry *reg) const = 0;
    virtual Object Duplicate(StorageBase const &storage) const = 0;
    virtual void Assign(StorageBase &, StorageBase const &) const = 0;
    virtual bool Equiv(const StorageBase &A, const StorageBase &B) const = 0;
    virtual bool Less(const StorageBase &A, const StorageBase &B) const = 0;
    virtual bool Greater(const StorageBase &A, const StorageBase &B) const = 0;
    virtual void SetStringValue(StorageBase &, const String &) const = 0;
    virtual void Insert(StringStream &, StorageBase const &) const = 0;
    virtual StorageBase *Extract(Registry &, StringStream &) const = 0;
    virtual void ExtractValue(Object &object,
                              StringStream &strstream) const = 0;
    virtual HashValue GetHashValue(const StorageBase &) const = 0;
};

// Factory functions that return smart pointers
template <class Property>
std::unique_ptr<PropertyBase> MakeProperty(
    Property prop, const Label &label,
    MemberCreateParams::Enum create_params = MemberCreateParams::Default) {
    // Implementation would create the appropriate property wrapper
    // This is a template - actual implementation depends on property system
    return std::make_unique<PropertyWrapper<Property>>(prop, label,
                                                       create_params);
}

KAI_END
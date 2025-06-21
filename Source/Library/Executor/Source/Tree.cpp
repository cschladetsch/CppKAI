#include "KAI/Core/Tree.h"

#include <KAI/Core/Exception/Extended.h>
#include <KAI/Core/Object/GetStorageBase.h>
#include <KAI/Core/Registry.h>
#include <KAI/Core/Type/Deref.h>

#include <iostream>

#include "KAI/Core/BuiltinTypes/Void.h"

KAI_BEGIN

Pathname GetFullname(const Object &Q) {
    if (!Q.Valid()) return Pathname();

    return GetFullname(GetStorageBase(Q));
}

Pathname GetFullname(const StorageBase &obj) {
    std::list<String> parentage;
    if (auto const &label = obj.GetLabel(); !label.ToString().empty())
        parentage.push_back(label.ToString());

    for (auto parent = obj.GetParent(); parent.Valid();
         parent = parent.GetParent())
        parentage.push_front(GetStorageBase(parent).GetLabel().ToString());

    StringStream path;
    if (parentage.empty()) return Pathname("[anon]");
    path.Append('/');

    for (auto const &name : parentage) path.Append(name);

    return path.ToString();
}

void Set(Object scope, const Pathname &path, Object const &Q) {
    if (path.Empty()) return;

    auto A = path.begin(), B = --path.end();

    if (B->type != Pathname::Element::Name)
        KAI_THROW_1(InvalidPathname, path.ToString());

    for (; A != B; ++A) {
        switch (A->type) {
            case Pathname::Element::None:
                break;

            case Pathname::Element::Separator:
                break;

            case Pathname::Element::This:
                break;

            case Pathname::Element::Parent:
                scope = scope.GetParent();
                break;

            case Pathname::Element::Name: {
                const Label &name = A->name_;
                if (A == --path.end()) {
                    GetStorageBase(scope).Set(name, Q);
                    break;
                }

                StorageBase &base = GetStorageBase(scope);
                if (!base.Has(name)) {
                    Object added = scope.GetRegistry()->New<void>();
                    base.Set(name, added);
                    scope = added;
                } else {
                    scope = base.Get(name);
                }
            } break;
        }
    }

    GetStorageBase(scope).Set(A->name_, Q);
}

void Set(Object const &root, Object const &scope, Object const &ident,
         Object const &Q) {
    switch (GetTypeNumber(ident).value) {
        case Type::Number::Label: {
            const Label &label = ConstDeref<Label>(ident);
            GetStorageBase(scope).Set(label, Q);
            break;
        }

        case Type::Number::Pathname: {
            const Pathname &path = ConstDeref<Pathname>(ident);
            if (path.Absolute())
                Set(root, path, Q);
            else
                Set(scope, path, Q);
            break;
        }

        default:
            KAI_THROW_1(InvalidIdentifier, Q);
    }
}

Object Get(Object const &root, Object const &scope, const Pathname &path) {
    if (path.Empty()) return path.Absolute() ? root : Object();

    if (path.Absolute()) return Get(root, path);

    return Get(scope, path);
}

Object Get(Object scope, const Pathname &path) {
    if (!scope.Exists()) return Object();

    for (const auto &element : path.GetElements()) {
        switch (element.type) {
            case Pathname::Element::None:
                break;

            case Pathname::Element::Separator:
                break;

            case Pathname::Element::Name: {
                scope = scope.Get(element.name_);
                if (!scope.Exists()) return scope;
            } break;

            case Pathname::Element::This:
                break;

            case Pathname::Element::Parent:
                scope = scope.GetParent();
                break;
        }
    }

    return scope;
}

bool Exists(Object const &scope, const Pathname &path) {
    try {
        return Get(scope, path).Exists();
    } catch (const Exception::ObjectNotFound &e) {
        // Object not found is expected and not an error in Exists check
        KAI_TRACE() << "Path not found: " << path.ToString() << " - "
                    << e.ToString();
    } catch (const std::exception &e) {
        KAI_TRACE_ERROR() << "Error checking existence: " << e.what();
    } catch (...) {
        KAI_TRACE_ERROR() << "Unknown error checking existence";
    }

    return false;
}

bool Exists(Object const &root, Object const &scope, const Pathname &path) {
    try {
        return Get(root, scope, path).Exists();
    } catch (const Exception::ObjectNotFound &e) {
        // Object not found is expected and not an error in Exists check
        KAI_TRACE() << "Path not found: " << path.ToString() << " - "
                    << e.ToString();
    } catch (const std::exception &e) {
        KAI_TRACE_ERROR() << "Error checking existence: " << e.what();
    } catch (...) {
        KAI_TRACE_ERROR() << "Unknown error checking existence";
    }

    return false;
}

void Remove(Object scope, const Pathname &path) {
    if (path.Empty()) KAI_THROW_0(Base);

    const Pathname::Elements &elements = path.GetElements();
    if (elements.back().type != Pathname::Element::Name) KAI_THROW_0(Base);

    auto A = elements.begin(), B = elements.end();
    for (--B; A != B; ++A) {
        switch (A->type) {
            case Pathname::Element::None:
                break;

            case Pathname::Element::Separator:
                break;

            case Pathname::Element::Name:
                scope = scope.Get(A->name_);
                break;

            case Pathname::Element::This:
                break;

            case Pathname::Element::Parent:
                scope = scope.GetParent();
                break;
        }
    }

    scope.Remove(A->name_);
}

void Remove(Object const &root, Object const &scope, const Pathname &path) {
    if (path.Absolute())
        Remove(root, path);
    else
        Remove(scope, path);
}

void Remove(Object const &root, Object const &scope, Object const &ident) {
    const StorageBase &id = ident.GetStorageBase();
    if (id.GetTypeNumber() == Type::Traits<Label>::Number) {
        scope.Remove(ConstDeref<Label>(id));
        return;
    }

    Remove(root, scope, ConstDeref<Pathname>(id));
}

Label GetName(const Object &object) {
    if (!object.Exists()) return Label();

    return object.GetLabel();
}

void Tree::SetScope(const Object &object) {
    Pathname path = GetFullname(object);
    if (!Exists(root_, scope_, path)) KAI_THROW_1(Base, "ObjectNotInTree");

    scope_ = object;
}

void Tree::SetScope(const Pathname &path) {
    if (!Exists(root_, scope_, path)) KAI_THROW_0(ObjectNotFound);

    scope_ = Get(root_, scope_, path);
}

void Tree::AddSearchPath(const Object &path) {
    if (!path.Exists()) {
        KAI_TRACE_ERROR() << "AddSearchPath with invalid object";
        return;
    }
    path_.push_back(path);
}

void Tree::AddSearchPath(const Pathname &path) {
    if (path.Empty()) {
        KAI_TRACE_ERROR() << "AddSearchPath with empty pathname";
        return;
    }
    Object found = Get(root_, scope_, path);
    if (!found.Exists()) {
        KAI_TRACE_ERROR() << "AddSearchPath failed to resolve: "
                          << path.ToString();
        return;
    }
    path_.push_back(found);
}

Object Tree::Resolve(const Label &label) const {
    if (scope_.Exists()) {
        if (Object found = scope_.Get(label); found.Exists()) return found;
    }

    for (Object const &object : path_) {
        if (!object.Exists()) continue;

        if (Object found = object.Get(label); found.Exists()) return found;
    }

    return Object();
}

Object Tree::Resolve(const Pathname &pathName) const {
    if (Object found = Get(root_, scope_, pathName); found.Exists())
        return found;

    if (pathName.Absolute()) return Object();

    for (auto const &scope : path_) {
        if (Object found = Get(root_, scope, pathName); found.Exists())
            return found;
    }

    return Object();
}

KAI_END

// EOF
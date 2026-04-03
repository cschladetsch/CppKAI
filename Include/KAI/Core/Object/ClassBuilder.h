#pragma once

#include <KAI/Core/SetGCFlagFwd.h>

#include "KAI/Core/Object/Accessor.h"
#include "KAI/Core/Object/Class.h"
#include "KAI/Core/Object/Method.h"
#include "KAI/Core/Object/PropertyBase.h"
#include "KAI/Core/Pathname.h"

#undef RegisterClass

KAI_BEGIN

void RegisterClass(Registry &, ClassBase const &, Object const &,
                   Pathname const &);

template <class T>
class ClassBuilder {
   public:
    typedef Type::Traits<T> Traits;
    Registry *registry;
    Class<T> *klass;
    Pathname path;
    Object root;

    struct MethodsCollector {
        struct PropertiesCollector {
            ClassBuilder<T> *builder;

            PropertiesCollector &operator=(PropertiesCollector &);

            template <class Property>
            PropertiesCollector &operator()(
                const char *N, Property P, String const &D = "",
                MemberCreateParams::Enum create_params =
                    MemberCreateParams::Default) {
                auto label = Label(N);
                auto Q = MakeProperty<T>(P, label, create_params);
                if (!D.empty()) Q->Description = D;
                builder->klass->AddProperty(label, Q.release());
                return *this;
            }
        };

        ClassBuilder<T> *builder;
        PropertiesCollector Properties;

        MethodsCollector &operator=(MethodsCollector &);

        template <class Method>
        MethodsCollector &operator()(const char *name, Method method,
                                     String const &D = "") {
            auto label = Label(name);
            auto M = MakeMethod(method, label);
            if (!D.empty()) M->Description = D;
            builder->klass->AddMethod(label, M.release());
            return *this;
        }
    };

    MethodsCollector Methods;

    ClassBuilder(Registry &R, const char *N)
        : registry(&R), klass(new Class<T>(Label(N))) {
        Methods.builder = this;
        Methods.Properties.builder = this;
    }

    ClassBuilder(Registry &R, Label const &N)
        : registry(&R), klass(new Class<T>(N)) {
        Methods.builder = this;
        Methods.Properties.builder = this;
    }

    ClassBuilder(Registry &R, const Label &N, Object const &Q,
                 const Pathname &P)
        : registry(&R), root(Q), path(P), klass(new Class<T>(N)) {
        Methods.builder = this;
        Methods.Properties.builder = this;
    }

    ~ClassBuilder() { RegisterClass(*registry, *klass, root, path); }
};

KAI_END

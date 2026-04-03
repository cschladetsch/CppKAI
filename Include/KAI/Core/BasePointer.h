#pragma once

#include <KAI/Core/BasePointerBase.h>
#include <KAI/Core/Config/Base.h>
#include <KAI/Core/Type.h>

KAI_BEGIN

template <class T>
struct BasePointer : BasePointerBase, FwdBasePointer<T> {
    typedef T Base;
    typedef typename Type::Traits<Base> BaseTraits;
    std::shared_ptr<Base> base;

    BasePointer(Base *P = 0) : base(P) {}

    Base *GetBase() const { return &*base; }
    Base *operator->() const { return GetBase(); }
    Base &operator*() const { return *GetBase(); }

    static void Register(Registry &R) {
        KAI_UNUSED(R);

        /* MUSTFIX
        ClassBuilder<BasePointer<T> >(R, String("BasePointer"))
            ;
            */
    }
};

KAI_TYPE_TRAITS(BasePointerBase, Number::BasePointer,
                Properties::StringStreamInsert);

KAI_END

#pragma once

#include <KAI/Core/Registry.h>
#include <KAI/Core/Value.h>
#include <KAI/Language/Common/Process.h>

KAI_BEGIN

struct ProcessCommon : Process {
    template <class T>
    Value<T> New() {
        return reg_->New<T>();
    }

    template <class T>
    Value<T> New(const T &val) {
        return reg_->New<T>(val);
    }

    ProcessCommon() {}
    ProcessCommon(Registry &r) : reg_(&r) {}

   protected:
    Registry *reg_ = 0;
};

KAI_END

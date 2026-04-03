#pragma once

#include <KAI/Core/Config/Base.h>

#include "Constness.h"
#include "Handle.h"

KAI_BEGIN

struct ObjectConstructParams {
    const ClassBase *class_base;
    Registry *registry;
    Handle handle;
    Constness constness;

    ObjectConstructParams() : class_base(0), registry(0) {}
    ObjectConstructParams(Registry *, const ClassBase *, Handle,
                          Constness = Constness::Mutable);
    ObjectConstructParams(const Object &, Constness);
    ObjectConstructParams(const StorageBase *);
    ObjectConstructParams(StorageBase *);
};

KAI_END

#pragma once

#include <KAI/Core/Config/Base.h>
#include <KAI/Core/Type/TraitMacros.h>

#include <map>

#include "KAI/Core/BuiltinTypes/MapBase.h"
#include "KAI/Core/StringStream.h"

KAI_BEGIN

struct MapCompare {
    bool operator()(const Object &A, const Object &B) const;
};

typedef MapBase<std::map<Object, Object, MapCompare> > Map;

KAI_TYPE_TRAITS(Map, Number::Map,
                Properties::StringStreamInsert  // TODO: StringStreaming
                    | Properties::BinaryStreaming | Properties::Equiv |
                    Properties::Reflected | Properties::Container
                // Properties::Less ??
);

KAI_END

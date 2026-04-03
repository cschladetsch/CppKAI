#pragma once

#include <KAI/Core/Config/Base.h>

KAI_BEGIN

///
/// Used for tri-color garbage collection
///
/// see also
/// http://stackoverflow.com/questions/2364274/tri-color-incremental-updating-gc-does-it-need-to-scan-each-stack-twice
///
struct ObjectColor {
    /// All If we have no Grey objects, all objects are either White or Black.
    /// In that case, we can collect the White objects.
    enum Color {
        /// All objects start White.
        /// White objects are ready for collection.
        White,

        /// Grey objects are reachable from Root of Tree and/or Executor
        /// Context(s) For each Grey object, find all references to White
        /// objects and turn them Grey. After that, we turn ourself Black
        Grey,

        /// Take all Grey objects, find all references it has to White objects
        /// and turn them Grey. Then we color that each _root object Black.
        Black
    };
};

KAI_END

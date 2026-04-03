#pragma once

#include "TestCommon.h"

KAI_BEGIN

class MyStruct {
   public:
    // this is accessible to the runtime after registration below
    int num;
    String string;

    // the Pointer<> wrapper makes this field a KAI object, which means it will
    // be subject to garbage collection
    Pointer<List> list;

    mutable bool called[3];

    int Method0() {
        called[0] = true;
        return 42;
    }

    String Method1(int n, String str) const {
        called[1] = true;
        KAI_TRACE_1(str);
        String result;
        while (n-- > 0) result += str;
        return result;
    }

    void Method2() { called[2] = true; }

    // every class that is to be reflected must be registered.
    // by convention, this is done in a static function called Register
    static void Register(Registry &);
};

// register the type
KAI_TYPE_TRAITS(MyStruct, 555, Properties::Nothing);

KAI_END

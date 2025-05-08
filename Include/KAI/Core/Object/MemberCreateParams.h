#pragma once

#include <KAI/Core/Config/Base.h>

KAI_BEGIN

namespace MemberCreateParams {
enum Enum {
    Create = 1,

    //< when supplied to a ClassBuilder<>'s property list, this indicates that
    // the associated property will < not be created when the owning object
    // instance is created. for example:
    /*
    struct Foo
    {
        Pointer<Bar> bar;
    };
    ClassBuilder<Foo>("Foo", R)
        .Methods
        .Properties
            ("bar", &Foo::bar, "a bar")
        ;
    // in this case, when a new Foo object is created via Pointer<Foo> foo =
    New<Foo>(), the
    // encapsulated bar object will also be created via New<Bar>.
    //
    // however, if the `bar` property was added using the line:
    //        ("bar", &Foo::bar, "a bar", Create::NoDefault)
    //                                    ^^^^^^^^^^^^^^^^^
    // then the `bar` object in a new foo will be Null.
    */
    NoCreate = 2,

    // default to no creation
    Default = NoCreate,
};
}

KAI_END

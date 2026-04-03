#include "MyTestStruct.h"

#include "TestCommon.h"

KAI_BEGIN

// this is a C++ struct that we will reflect to access at runtime via scripting
// note that we do not have to derive from any magic base classes_
// note as well that there is no need for magic macros in the class definition
// this also means that we can reflect any C++ class, even if we do not
// have access to its source code. no recompile is needed.
// register MyStruct with a registry. it couldn't be easier.
void MyStruct::Register(Registry &registry) {
    ClassBuilder<MyStruct>(registry, "MyStruct")
        .Methods("Method0", &MyStruct::Method0, "A method")(
            "Method1", &MyStruct::Method1, "Another method")(
            "Method2", &MyStruct::Method2, "Another method")
        .Properties("num", &MyStruct::num, "a number")(
            "string", &MyStruct::string, "a string")
        //("list", &MyStruct::list, "a list")
        ;
}

KAI_END

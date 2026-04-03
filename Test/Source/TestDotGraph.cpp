#include "KAI/Core/BuiltinTypes.h"
#include "KAI/Core/DotGraph.h"
#include "KAI/Core/Object/ClassBuilder.h"
#include "KAI/Core/Tree.h"
#include "TestCommon.h"

using namespace kai;
using namespace std;

struct TestOwned {
    Pointer<List> list;
    Pointer<int> num;
};

KAI_BEGIN

KAI_TYPE_TRAITS(TestOwned, 666, 0);

KAI_END

TEST(TestDotGraph, Test) {
    Registry R;
    R.AddClass<void>();
    R.AddClass<int>();
    R.AddClass<List>();
    ClassBuilder<TestOwned>(R, "TestOwned")
        .Methods.Properties
        //("list", &TestOwned::list, "", CreateParams::Create)
        ("num", &TestOwned::num);

    Tree tree;
    Object root = R.New<void>();
    R.SetTree(tree);

    Pointer<TestOwned> to = R.New<TestOwned>();
    Pointer<int> n = R.New<int>();

    root.SetChild(Label("test_owned"), to);
    root.SetChild(Label("n"), n);

    to->list = R.New<List>();
    to->list->PushBack(R.New(42));
    to->list->PushBack(R.New(123));
    to->num = R.New(456);

    String filename("test.graph");
    DotGraph(root, filename);
}

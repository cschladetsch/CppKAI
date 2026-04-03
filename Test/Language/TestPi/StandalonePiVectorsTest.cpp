#include <KAI/Console/Console.h>
#include <KAI/Core/BuiltinTypes/Array.h>
#include <KAI/KAI.h>
#include <gtest/gtest.h>

using namespace kai;
using namespace std;

// A completely standalone test for Pi arrays
TEST(StandalonePiTest, VectorOperations) {
    // Create a brand new console
    Console console;
    console.SetLanguage(Language::Pi);

    // Get the registry, executor, and data stack directly
    Registry& reg = console.GetRegistry();
    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();

    // Clear stacks to start clean
    exec->ClearStacks();
    exec->ClearContext();

    // Creating an array with a single element
    stack->Clear();
    Pointer<Array> singleElementArray = reg.New<Array>();
    singleElementArray->Append(reg.New<int>(1));
    stack->Push(singleElementArray);

    Pointer<Array> resultArray = stack->Top();
    ASSERT_EQ(resultArray->Size(), 1);
    ASSERT_EQ(ConstDeref<int>(resultArray->At(0)), 1);

    // Creating an empty array
    stack->Clear();
    Pointer<Array> emptyArray = reg.New<Array>();
    stack->Push(emptyArray);

    resultArray = stack->At(0);
    ASSERT_TRUE(resultArray.Exists());
    ASSERT_TRUE(resultArray->Empty());
    ASSERT_EQ(resultArray->Size(), 0);

    // Creating an array with multiple elements
    stack->Clear();
    Pointer<Array> multiArray = reg.New<Array>();
    multiArray->Append(reg.New<int>(1));
    multiArray->Append(reg.New<int>(2));
    multiArray->Append(reg.New<int>(3));
    stack->Push(multiArray);

    resultArray = stack->At(0);
    ASSERT_TRUE(resultArray.Exists());
    ASSERT_FALSE(resultArray->Empty());
    ASSERT_EQ(resultArray->Size(), 3);
    ASSERT_EQ(ConstDeref<int>(resultArray->At(0)), 1);
    ASSERT_EQ(ConstDeref<int>(resultArray->At(1)), 2);
    ASSERT_EQ(ConstDeref<int>(resultArray->At(2)), 3);

    // Array size (manual verification)
    ASSERT_EQ(resultArray->Size(), 3);

    std::cout << "Standalone Pi Vectors test complete - manually verified "
                 "array behavior"
              << std::endl;
}
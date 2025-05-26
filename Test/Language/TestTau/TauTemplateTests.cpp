#include <gtest/gtest.h>
#include "TestLangCommon.h"

// Test suite for Tau template and generic programming
TEST(TauTemplate, BasicTemplateClass) {
    kai::Console console;
    console.SetLanguage(kai::Language::Tau);
    
    const char* code = R"(
        template<T>
        class Container {
            T[] items;
            
            void Add(T item) {
                items.push(item);
            }
            
            T Get(int index) {
                return items[index];
            }
            
            int Size() {
                return items.size();
            }
        }
        
        // Instantiate with int
        Container<int> intContainer;
        intContainer.Add(42);
        intContainer.Get(0);
    )";
    
    console.Execute(code);
    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 42);
}

TEST(TauTemplate, TemplateFunction) {
    kai::Console console;
    console.SetLanguage(kai::Language::Tau);
    
    const char* code = R"(
        template<T>
        T Max(T a, T b) {
            return a > b ? a : b;
        }
        
        int result1 = Max<int>(5, 10);
        float result2 = Max<float>(3.14, 2.71);
        result1 + (int)result2;
    )";
    
    console.Execute(code);
    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 13); // 10 + 3
}

TEST(TauTemplate, TemplateSpecialization) {
    kai::Console console;
    console.SetLanguage(kai::Language::Tau);
    
    const char* code = R"(
        template<T>
        class TypeInfo {
            static string GetName() {
                return "Generic";
            }
        }
        
        // Specialization for int
        template<>
        class TypeInfo<int> {
            static string GetName() {
                return "Integer";
            }
        }
        
        TypeInfo<int>::GetName();
    )";
    
    console.Execute(code);
    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<kai::String>(stack->Top()), "Integer");
}

TEST(TauTemplate, VariadicTemplates) {
    kai::Console console;
    console.SetLanguage(kai::Language::Tau);
    
    const char* code = R"(
        template<typename... Args>
        int Sum(Args... args) {
            return (... + args);
        }
        
        Sum(1, 2, 3, 4, 5);
    )";
    
    console.Execute(code);
    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 15);
}

TEST(TauTemplate, ConceptConstraints) {
    kai::Console console;
    console.SetLanguage(kai::Language::Tau);
    
    const char* code = R"(
        concept Numeric<T> {
            requires T::operator+(T, T) -> T;
            requires T::operator*(T, T) -> T;
        }
        
        template<T> where Numeric<T>
        T Square(T value) {
            return value * value;
        }
        
        Square(7);
    )";
    
    console.Execute(code);
    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 49);
}
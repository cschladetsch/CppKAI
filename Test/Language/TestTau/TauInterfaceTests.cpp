#include <gtest/gtest.h>
#include "TestLangCommon.h"

// Test suite for Tau interfaces and contracts
TEST(TauInterface, BasicInterface) {
    kai::Console console;
    console.SetLanguage(kai::Language::Tau);
    
    const char* code = R"(
        interface IShape {
            float Area();
            float Perimeter();
        }
        
        class Rectangle : IShape {
            float width, height;
            
            Rectangle(float w, float h) {
                width = w;
                height = h;
            }
            
            float Area() {
                return width * height;
            }
            
            float Perimeter() {
                return 2 * (width + height);
            }
        }
        
        IShape shape = Rectangle(4.0, 5.0);
        shape.Area();
    )";
    
    console.Execute(code);
    auto &exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_FLOAT_EQ(kai::ConstDeref<float>(stack->Top()), 20.0f);
}

TEST(TauInterface, MultipleInterfaces) {
    kai::Console console;
    console.SetLanguage(kai::Language::Tau);
    
    const char* code = R"(
        interface IDrawable {
            void Draw();
        }
        
        interface IMovable {
            void Move(int dx, int dy);
        }
        
        class Sprite : IDrawable, IMovable {
            int x, y;
            
            Sprite() {
                x = 0;
                y = 0;
            }
            
            void Draw() {
                // Drawing logic
            }
            
            void Move(int dx, int dy) {
                x += dx;
                y += dy;
            }
            
            int GetX() { return x; }
        }
        
        Sprite s;
        IMovable m = s;
        m.Move(10, 20);
        s.GetX();
    )";
    
    console.Execute(code);
    auto &exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 10);
}

TEST(TauInterface, InterfaceInheritance) {
    kai::Console console;
    console.SetLanguage(kai::Language::Tau);
    
    const char* code = R"(
        interface IAnimal {
            string GetSpecies();
        }
        
        interface IMammal : IAnimal {
            int GetGestationPeriod();
        }
        
        class Dog : IMammal {
            string GetSpecies() {
                return "Canis familiaris";
            }
            
            int GetGestationPeriod() {
                return 63;
            }
        }
        
        IMammal mammal = Dog();
        mammal.GetGestationPeriod();
    )";
    
    console.Execute(code);
    auto &exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 63);
}

TEST(TauInterface, DefaultInterfaceMethods) {
    kai::Console console;
    console.SetLanguage(kai::Language::Tau);
    
    const char* code = R"(
        interface ILogger {
            void Log(string message);
            
            void LogError(string message) {
                Log("[ERROR] " + message);
            }
            
            void LogWarning(string message) {
                Log("[WARN] " + message);
            }
        }
        
        class ConsoleLogger : ILogger {
            int logCount = 0;
            
            void Log(string message) {
                logCount++;
            }
            
            int GetLogCount() { return logCount; }
        }
        
        ConsoleLogger logger;
        ILogger ilog = logger;
        ilog.LogError("Test");
        ilog.LogWarning("Test");
        logger.GetLogCount();
    )";
    
    console.Execute(code);
    auto &exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 2);
}

TEST(TauInterface, GenericInterfaces) {
    kai::Console console;
    console.SetLanguage(kai::Language::Tau);
    
    const char* code = R"(
        interface IContainer<T> {
            void Add(T item);
            T Get(int index);
            int Count();
        }
        
        class List<T> : IContainer<T> {
            T[] items;
            
            void Add(T item) {
                items.push(item);
            }
            
            T Get(int index) {
                return items[index];
            }
            
            int Count() {
                return items.size();
            }
        }
        
        IContainer<int> container = List<int>();
        container.Add(42);
        container.Get(0);
    )";
    
    console.Execute(code);
    auto &exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 42);
}
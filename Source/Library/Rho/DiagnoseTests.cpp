#include <KAI/Core/BuiltinTypes.h>
#include <KAI/Core/Console.h>

#include <iostream>

using namespace kai;
using namespace std;

// This is a simple test harness to diagnose the issue with Continuations
int main(int argc, char** argv) {
    // Create a console with Pi language
    Console console;
    console.SetLanguage(Language::Pi);

    // Register required types
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<String>(Label("String"));
    reg.AddClass<bool>(Label("bool"));
    reg.AddClass<Continuation>(Label("Continuation"));

    // Get executor and set trace level
    auto exec = console.GetExecutor();
    exec->SetTraceLevel(3);  // Maximum trace level

    try {
        // Test 1: Simple arithmetic
        cout << "Test 1: Simple arithmetic (2 + 3)..." << endl;
        exec->GetDataStack()->Clear();
        console.Execute("2 3 +");

        // Check the stack
        auto stack = exec->GetDataStack();
        if (stack->Empty()) {
            cout << "ERROR: Stack is empty" << endl;
        } else {
            Object result = stack->Top();
            cout << "Result type: " << result.GetClass()->GetName() << endl;
            cout << "Result value: " << result.ToString() << endl;

            // Try to extract the value if it's an int
            if (result.IsType<int>()) {
                cout << "Integer result: " << ConstDeref<int>(result) << endl;
            } else if (result.IsType<Continuation>()) {
                cout << "Continuation result, analyzing contents..." << endl;
                Pointer<Continuation> cont = result;
                if (cont->GetCode().Exists()) {
                    cout << "Continuation code size: "
                         << cont->GetCode()->Size() << endl;
                    for (int i = 0; i < cont->GetCode()->Size(); ++i) {
                        auto item = cont->GetCode()->At(i);
                        cout << "  Item " << i << ": " << item.ToString()
                             << " (Type: " << item.GetClass()->GetName() << ")"
                             << endl;
                    }
                } else {
                    cout << "Continuation has no code" << endl;
                }
            }
        }

        // Test 2: Simple statement with Rho
        cout << "\nTest 2: Simple Rho statement (2 + 3)..." << endl;
        exec->GetDataStack()->Clear();
        console.SetLanguage(Language::Rho);
        console.Execute("2 + 3");

        // Check the stack
        stack = exec->GetDataStack();
        if (stack->Empty()) {
            cout << "ERROR: Stack is empty" << endl;
        } else {
            Object result = stack->Top();
            cout << "Result type: " << result.GetClass()->GetName() << endl;
            cout << "Result value: " << result.ToString() << endl;

            // Try to extract the value if it's an int
            if (result.IsType<int>()) {
                cout << "Integer result: " << ConstDeref<int>(result) << endl;
            } else if (result.IsType<Continuation>()) {
                cout << "Continuation result, analyzing contents..." << endl;
                Pointer<Continuation> cont = result;
                if (cont->GetCode().Exists()) {
                    cout << "Continuation code size: "
                         << cont->GetCode()->Size() << endl;
                    for (int i = 0; i < cont->GetCode()->Size(); ++i) {
                        auto item = cont->GetCode()->At(i);
                        cout << "  Item " << i << ": " << item.ToString()
                             << " (Type: " << item.GetClass()->GetName() << ")"
                             << endl;
                    }
                } else {
                    cout << "Continuation has no code" << endl;
                }
            }
        }

        return 0;
    } catch (const Exception::Base& e) {
        cerr << "KAI Exception: " << e.ToString() << endl;
        return 1;
    } catch (const std::exception& e) {
        cerr << "Standard exception: " << e.what() << endl;
        return 1;
    } catch (...) {
        cerr << "Unknown exception" << endl;
        return 1;
    }
}
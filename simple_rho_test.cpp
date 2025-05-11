#include <iostream>
#include "Include/KAI/Core/Console.h"
#include "Include/KAI/Executor/Executor.h"

using namespace kai;
using namespace std;

int main() {
    Console console;
    console.SetLanguage(Language::Rho);
    
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<String>(Label("String"));
    reg.AddClass<bool>(Label("bool"));
    
    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    
    // Execute a simple arithmetic expression
    try {
        cout << "Executing expression: 2 + 3" << endl;
        console.Execute("2 + 3");
        
        if (!stack->Empty()) {
            Object result = stack->Top();
            if (result.IsType<int>()) {
                cout << "Result: " << ConstDeref<int>(result) << endl;
            } else {
                cout << "Unexpected result type: " << result.GetClass()->GetTypeNumber() << endl;
            }
        } else {
            cout << "Stack is empty!" << endl;
        }
    }
    catch (const Exception::Base& e) {
        cerr << "KAI Exception: " << e.ToString() << endl;
        return 1;
    }
    catch (const std::exception& e) {
        cerr << "Standard exception: " << e.what() << endl;
        return 1;
    }
    catch (...) {
        cerr << "Unknown exception" << endl;
        return 1;
    }
    
    return 0;
}
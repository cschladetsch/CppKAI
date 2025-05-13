#include <iostream>
#include <string>

#include "KAI/Console/Console.h"

using namespace kai;
using namespace std;

int main() {
    // Create console and set to Rho language
    Console console;
    console.SetLanguage(Language::Rho);
    
    // Register basic types
    Registry& reg = console.GetRegistry();
    
    cout << "Attempting to execute Rho assertion code: assert(1+1==2)" << endl;
    
    // Execute the Rho code with assertion
    try {
        console.Execute("assert(1+1==2);");
        cout << "Assertion passed successfully!" << endl;
        
        // Print stack contents
        auto stack = console.GetExecutor()->GetDataStack();
        cout << "Stack size: " << stack->Size() << endl;
        
        // Try a failing assertion to verify assert is working
        cout << "\nTrying a failing assertion: assert(1+1==3)" << endl;
        try {
            console.Execute("assert(1+1==3);");
            cout << "ERROR: This should have failed!" << endl;
        }
        catch (const std::exception& e) {
            cout << "Expected failure happened: " << e.what() << endl;
        }
    }
    catch (const std::exception& e) {
        cout << "Execution failed with error: " << e.what() << endl;
    }
    
    return 0;
}
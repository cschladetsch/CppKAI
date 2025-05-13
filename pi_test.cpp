#include <iostream>
#include <string>
#include <fstream>

#include "KAI/Console/Console.h"

using namespace kai;
using namespace std;

int main() {
    // Create console and set to Pi language
    Console console;
    console.SetLanguage(Language::Pi);
    
    // Register basic types
    Registry& reg = console.GetRegistry();
    
    // Read the Pi code from file
    ifstream file("pi_test.txt");
    string code((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();
    
    cout << "Executing Pi code: " << code << endl;
    
    // Execute the Pi code
    try {
        console.Execute(code);
        cout << "Execution successful!" << endl;
        
        // Print stack contents
        auto stack = console.GetExecutor()->GetDataStack();
        cout << "Stack size: " << stack->Size() << endl;
        
        if (stack->Size() > 0) {
            cout << "Stack top value: " << stack->Top().ToString() << endl;
        }
    }
    catch (const std::exception& e) {
        cout << "Execution failed with error: " << e.what() << endl;
    }
    
    return 0;
}
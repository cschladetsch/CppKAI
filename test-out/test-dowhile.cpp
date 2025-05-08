#include <iostream>
#include <string>
#include <fstream>
#include "KAI/Core/Console.h"

using namespace KAI;
using namespace std;

int main() {
    // Initialize the console with Rho language
    Console console;
    console.SetLanguage(Language::Rho);
    
    // Set high trace level for debugging
    console.GetExecutor()->SetTraceLevel(5);
    
    try {
        // Read the test file
        string filename = "/home/xian/local/KAI/test-out/do-while-test.rho";
        ifstream file(filename);
        if (!file) {
            cerr << "Failed to open file: " << filename << endl;
            return 1;
        }
        
        // Read the whole file into a string
        string script((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        file.close();
        
        cout << "Executing script:" << endl;
        cout << "----------------" << endl;
        cout << script << endl;
        cout << "----------------" << endl;
        
        // Execute the script
        console.Execute(script);
        
        // Check results (should have i=3 on stack)
        auto stack = console.GetExecutor()->GetDataStack();
        cout << "Stack size after execution: " << stack->Size() << endl;
        
        cout << "Test completed successfully." << endl;
        return 0;
    }
    catch (Exception::Base &e) {
        cerr << "KAI Exception: " << e.ToString() << endl;
    }
    catch (std::exception &e) {
        cerr << "Standard Exception: " << e.what() << endl;
    }
    catch (...) {
        cerr << "Unknown Exception" << endl;
    }
    
    return 1;
}
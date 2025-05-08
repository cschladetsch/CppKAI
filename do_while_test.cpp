#include <iostream>
#include <string>
#include <KAI/Core/Console.h>

using namespace KAI;
using namespace std;

int main() {
    // Create registry and console
    Registry reg;
    Tree tree(reg);
    Object root = tree.Root();
    Console console(reg, tree, root);
    
    // Set to Rho language
    console.SetLanguage(Language::Rho);
    
    // Set trace level to max for debugging
    console.GetExecutor()->SetTraceLevel(5);
    
    try {
        cout << "Testing do-while loop..." << endl;
        
        // Define the script
        string script = 
            "// Simple do-while test\n"
            "i = 0\n"
            "do\n"
            "    i = i + 1\n"
            "while i < 3\n"
            "assert(i == 3)\n";
            
        cout << "Script:" << endl;
        cout << script << endl;
        
        // Execute the script
        console.Execute(script);
        
        // Check the result
        auto stack = console.GetExecutor()->GetDataStack();
        cout << "Stack size after execution: " << stack->Size() << endl;
        
        cout << "Test completed successfully!" << endl;
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
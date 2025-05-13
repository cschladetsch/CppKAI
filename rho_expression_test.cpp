#include <iostream>
#include <KAI/Console/Console.h>

using namespace std;
using namespace kai;

int main() {
    // Create a console
    Console console;
    
    // Set language to Rho
    console.SetLanguage(Language::Rho);
    
    // Test a simple arithmetic expression
    cout << "Testing Rho arithmetic expression: 1 + 2 * 3" << endl;
    console.Process("1 + 2 * 3");
    
    // Test a simple assertion
    cout << "Testing Rho assertion: assert(1 + 1 == 2)" << endl;
    console.Process("assert(1 + 1 == 2)");
    
    // Test a more complex expression
    cout << "Testing complex Rho expression: (1 + 2) * (3 + 4)" << endl;
    console.Process("(1 + 2) * (3 + 4)");
    
    // Test a variable assignment and use
    cout << "Testing Rho variable assignment and use" << endl;
    console.Process("x = 10");
    console.Process("y = 5");
    console.Process("z = x + y");
    console.Process("assert(z == 15)");
    
    // Test with Pi language for comparison
    cout << "\nSwitching to Pi language for comparison..." << endl;
    console.SetLanguage(Language::Pi);
    
    // Test Pi assertion
    cout << "Testing Pi assertion: 1 1 + 2 == assert" << endl;
    console.Process("1 1 + 2 == assert");
    
    cout << "Tests completed" << endl;
    return 0;
}
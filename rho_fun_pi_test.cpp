#include <iostream>
#include <KAI/Console/Console.h>

using namespace std;
using namespace kai;

int main() {
    // Create a console
    Console console;
    
    // Set language to Rho
    console.SetLanguage(Language::Rho);
    
    // Test 1: Function definition and usage
    cout << "Test 1: Function definition with braces in Rho" << endl;
    console.Process("fun square(x) { return x * x; }");
    console.Process("result = square(5);");
    cout << "Result on stack: " << console.WriteStack() << endl;
    
    // Test 2: Pi block in Rho
    cout << "\nTest 2: Pi block in Rho" << endl;
    console.Process("pi{ 3 4 + }");
    cout << "Result on stack after Pi block: " << console.WriteStack() << endl;
    
    // Test 3: Combining Pi and Rho
    cout << "\nTest 3: Using Pi result in Rho" << endl;
    console.Process("y = pi{ 10 2 * };");
    console.Process("z = y + 5;");
    cout << "Final stack: " << console.WriteStack() << endl;
    
    // Test 4: Function with Pi inside
    cout << "\nTest 4: Function with Pi block inside" << endl;
    console.Process("fun calculate(x) { return x + pi{ 3 4 + }; }");
    console.Process("result2 = calculate(10);");
    cout << "Final result: " << console.WriteStack() << endl;
    
    return 0;
}
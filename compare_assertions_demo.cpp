#include <iostream>
#include <string>

#include "KAI/Console/Console.h"

using namespace kai;
using namespace std;

void testPiAssertion() {
    cout << "\n=== TESTING PI LANGUAGE ASSERTIONS ===\n" << endl;
    
    Console console;
    console.SetLanguage(Language::Pi);
    
    // In Pi language, we use postfix notation
    // The assertion statement is: 1 1 + 2 assert
    // This pushes 1, pushes 1, adds them to get 2, pushes 2, and then checks they're equal
    
    cout << "Testing successful Pi assertion: '1 1 + 2 assert'" << endl;
    try {
        console.Execute("1 1 + 2 assert");
        cout << "→ Assertion passed!" << endl;
    }
    catch (const exception& e) {
        cout << "→ ERROR: Assertion unexpectedly failed: " << e.what() << endl;
    }
    
    cout << "\nTesting failing Pi assertion: '1 1 + 3 assert'" << endl;
    try {
        console.Execute("1 1 + 3 assert");
        cout << "→ ERROR: This should have failed!" << endl;
    }
    catch (const exception& e) {
        cout << "→ Assertion correctly failed with: " << e.what() << endl;
    }
}

void testRhoAssertion() {
    cout << "\n=== TESTING RHO LANGUAGE ASSERTIONS ===\n" << endl;
    
    Console console;
    console.SetLanguage(Language::Rho);
    
    // In Rho language, we use C-like infix notation
    // The assertion statement is: assert(1+1==2);
    
    cout << "Testing successful Rho assertion: 'assert(1+1==2);'" << endl;
    try {
        console.Execute("assert(1+1==2);");
        cout << "→ Assertion passed!" << endl;
    }
    catch (const exception& e) {
        cout << "→ ERROR: Assertion unexpectedly failed: " << e.what() << endl;
    }
    
    cout << "\nTesting failing Rho assertion: 'assert(1+1==3);'" << endl;
    try {
        console.Execute("assert(1+1==3);");
        cout << "→ ERROR: This should have failed!" << endl;
    }
    catch (const exception& e) {
        cout << "→ Assertion correctly failed with: " << e.what() << endl;
    }
}

int main() {
    // First test Pi language assertions
    testPiAssertion();
    
    // Then test Rho language assertions
    testRhoAssertion();
    
    return 0;
}
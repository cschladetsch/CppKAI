#include <iostream>
#include <KAI/Console/Console.h>

using namespace std;
using namespace kai;

// Helper function to test an expression and report result
void testExpression(Console& console, const std::string& expression) {
    cout << "Testing Rho expression: " << expression << endl;
    
    try {
        console.Process(expression);
        cout << "  Result: Success" << endl;
    }
    catch (Exception::Base& e) {
        cout << "  Result: Exception - " << e.ToString() << endl;
    }
    catch (std::exception& e) {
        cout << "  Result: std::exception - " << e.what() << endl;
    }
    catch (...) {
        cout << "  Result: Unknown exception" << endl;
    }
    
    cout << endl;
}

int main() {
    // Create a console with Rho language
    Console console;
    console.SetLanguage(Language::Rho);
    
    cout << "=== Testing Rho Language Expression Handling ===" << endl << endl;
    
    // Basic expressions
    testExpression(console, "1 + 1");
    testExpression(console, "2 * 3");
    testExpression(console, "10 / 2");
    testExpression(console, "7 % 4");
    
    // Comparisons
    testExpression(console, "5 > 3");
    testExpression(console, "5 < 8");
    testExpression(console, "5 == 5");
    testExpression(console, "5 != 6");
    
    // Boolean logic
    testExpression(console, "true && true");
    testExpression(console, "true || false");
    testExpression(console, "!false");
    
    // Assertions
    testExpression(console, "assert(1 + 1 == 2)");
    testExpression(console, "assert(2 * 3 == 6)");
    testExpression(console, "assert(10 / 2 == 5)");
    
    // Variables
    testExpression(console, "x = 10");
    testExpression(console, "y = 20");
    testExpression(console, "z = x + y");
    testExpression(console, "assert(z == 30)");
    
    // Complex expressions
    testExpression(console, "(1 + 2) * (3 + 4)");
    testExpression(console, "a = 5; b = 10; c = a + b; assert(c == 15)");
    
    // Failing assertion (should cause an error)
    testExpression(console, "assert(1 + 1 == 3)");
    
    // Compare with Pi for reference
    cout << "=== Testing Pi Language for Comparison ===" << endl << endl;
    console.SetLanguage(Language::Pi);
    
    testExpression(console, "1 1 +");  // Should put 2 on the stack
    testExpression(console, "1 1 + 2 == assert");  // Should succeed
    
    cout << "Tests completed" << endl;
    return 0;
}
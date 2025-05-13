#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>

#include "KAI/Console/Console.h"

using namespace kai;
using namespace std;

int main() {
    try {
        cout << "=== Rho Assertion Test ===" << endl;
        
        // Create a console with Rho language
        Console console;
        console.SetLanguage(Language::Rho);
        
        // Write a simple assertion test to a file
        ofstream testFile("rho_assert_test.rho");
        testFile << "// This is a simple test of Rho assertions\n";
        testFile << "assert(1+1==2);\n";
        testFile << "// This should pass silently\n";
        testFile.close();
        
        // Read the file for execution
        ifstream inFile("rho_assert_test.rho");
        string code((istreambuf_iterator<char>(inFile)), istreambuf_iterator<char>());
        inFile.close();
        
        cout << "\nExecuting code:\n" << code << endl;
        
        // Execute the valid assertion and catch any exceptions
        try {
            console.Execute(code);
            cout << "✓ Test passed: assert(1+1==2) executed successfully without errors" << endl;
        }
        catch (const std::exception& e) {
            cout << "✗ Test failed: assert(1+1==2) should not throw an exception" << endl;
            cout << "  Error: " << e.what() << endl;
            return 1;
        }
        
        // Now test a failing assertion
        cout << "\nExecuting code: assert(1+1==3);" << endl;
        
        // Execute the failing assertion and catch the expected exception
        try {
            console.Execute("assert(1+1==3);");
            cout << "✗ Test failed: assert(1+1==3) should throw an assertion exception" << endl;
            return 1;
        }
        catch (const std::exception& e) {
            cout << "✓ Test passed: assert(1+1==3) correctly threw an exception" << endl;
            cout << "  Exception: " << e.what() << endl;
        }
        
        cout << "\nAll tests passed!" << endl;
        return 0;
    }
    catch (const std::exception& e) {
        cout << "Unexpected error: " << e.what() << endl;
        return 1;
    }
}
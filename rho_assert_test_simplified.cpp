#include <iostream>
#include <string>
#include <fstream>

#include "KAI/Console/Console.h"

using namespace kai;
using namespace std;

int main() {
    // Create console and set to Rho language
    Console console;
    console.SetLanguage(Language::Rho);
    
    // Write a simple test file with Rho code that tests an assertion
    ofstream testFile("rho_assert_test.txt");
    testFile << "assert(1+1==2);" << endl;
    testFile.close();
    
    // Read the test file
    ifstream file("rho_assert_test.txt");
    string code((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();
    
    cout << "Executing Rho code: " << code << endl;
    
    try {
        // Execute the Rho code
        console.Execute(code);
        cout << "Assert passed! 1+1==2 is TRUE" << endl;
        
        // Now try a failing assertion
        console.Execute("assert(1+1==3);");
        cout << "ERROR: This should have failed!" << endl;
    }
    catch (const exception& e) {
        cout << "Expected exception: " << e.what() << endl;
        cout << "Assert check is working correctly!" << endl;
    }
    
    return 0;
}
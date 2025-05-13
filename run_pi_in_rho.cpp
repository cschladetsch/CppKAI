#include <iostream>
#include <fstream>
#include <string>
#include <KAI/Console/Console.h>

using namespace kai;
using namespace std;

int main() {
    // Create console in Rho language
    Console console;
    console.SetLanguage(Language::Rho);
    
    // Open the Rho file
    string filename = "pi_in_rho_test.rho";
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Failed to open file: " << filename << endl;
        return 1;
    }
    
    // Read the file content
    string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();
    
    cout << "Executing Rho file with content: " << endl;
    cout << content << endl;
    cout << "--------------------" << endl;
    
    try {
        // Execute the Rho code
        console.Process(content);
        cout << "Pi assertion in Rho executed successfully!" << endl;
    }
    catch (const std::exception& e) {
        cout << "Execution failed with error: " << e.what() << endl;
        return 1;
    }
    
    // Now try a failing assertion
    cout << "\nTrying a failing Pi assertion: pi { 1 1 + 3 == assert }" << endl;
    
    try {
        console.Process("pi { 1 1 + 3 == assert }");
        cout << "ERROR: Failing assertion did not throw an exception!" << endl;
    }
    catch (const std::exception& e) {
        cout << "Expected exception caught: " << e.what() << endl;
        cout << "The failing assertion correctly threw an exception!" << endl;
    }
    
    return 0;
}
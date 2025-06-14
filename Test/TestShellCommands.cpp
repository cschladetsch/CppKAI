#include <iostream>
#include <memory>
#include <string>

#include "KAI/Console/Console.h"

using namespace kai;
using namespace std;

int main() {
    try {
        Console console;
        console.SetLanguage(Language::Pi);

        cout << "Testing shell command syntax with $ prefix\n";
        cout << "==========================================\n\n";

        // Test 1: Simple echo
        cout << "Test 1: $ echo 'Hello from shell'\n";
        console.Process("$ echo 'Hello from shell'");
        cout << "\n";

        // Test 2: pwd command
        cout << "Test 2: $ pwd\n";
        console.Process("$ pwd");
        cout << "\n";

        // Test 3: ls command
        cout << "Test 3: $ ls -la | head -3\n";
        console.Process("$ ls -la | head -3");
        cout << "\n";

        // Test 4: Mix with Pi code
        cout << "Test 4: Pi code then shell\n";
        console.Process("1 2 + .");
        console.Process("$ echo 'Result was 3'");
        cout << "\n";

        // Test 5: Environment variable
        cout << "Test 5: $ echo \"User: $USER\"\n";
        console.Process("$ echo \"User: $USER\"");
        cout << "\n";

        // Test 6: Complex shell command
        cout << "Test 6: $ date '+%Y-%m-%d %H:%M:%S'\n";
        console.Process("$ date '+%Y-%m-%d %H:%M:%S'");
        cout << "\n";

        // Test 7: Test in Rho mode
        console.SetLanguage(Language::Rho);
        cout << "Test 7: Shell in Rho mode\n";
        console.Process("$ echo 'Shell works in Rho too!'");
        cout << "\n";

        cout << "All tests completed successfully!\n";

    } catch (const std::exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}
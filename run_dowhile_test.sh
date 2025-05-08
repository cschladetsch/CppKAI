#!/bin/bash
# Script to test do-while functionality

# Ensure we're in the KAI root directory
cd "$(dirname "$0")"

# Create a temporary test file
TEST_FILE="test_dowhile.rho"

cat > "$TEST_FILE" << 'EOF'
// Simple do-while test with proper indentation
i = 0
print("Starting test with i = " + i)
do
    i = i + 1
    print("Inside loop, i = " + i)
while i < 3
print("After loop, i = " + i)
assert(i == 3)
print("Test passed!")
EOF

echo "=== Testing do-while loop functionality ==="
echo "Test file content:"
cat "$TEST_FILE"
echo "=== Running test with Rho interpreter ==="

# Run the test with RhoRunner if it exists
if [ -f "./Bin/App/RhoRunner" ]; then
    ./Bin/App/RhoRunner "$TEST_FILE"
elif [ -f "./Bin/Test/TestRho" ]; then
    # Create a custom test
    TEST_CPP="test_dowhile.cpp"
    
    cat > "$TEST_CPP" << 'EOF'
#include <iostream>
#include <fstream>
#include <string>
#include <KAI/Core/Registry.h>
#include <KAI/Core/Console.h>
#include <KAI/Executor/Executor.h>
#include <KAI/Language/Common/Process.h>

using namespace KAI;
using namespace std;

int main(int argc, char** argv) {
    try {
        if (argc < 2) {
            cerr << "Usage: " << argv[0] << " <script.rho>" << endl;
            return 1;
        }
        
        // Read script file
        ifstream file(argv[1]);
        if (!file.is_open()) {
            cerr << "Failed to open file: " << argv[1] << endl;
            return 1;
        }
        
        string script((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        file.close();
        
        // Create registry and console
        Registry reg;
        Console console(reg);
        
        // Set language to Rho
        console.SetLanguage(Language::Rho);
        
        // Set trace level
        console.GetExecutor()->SetTraceLevel(5);
        
        // Execute script
        cout << "Executing script..." << endl;
        console.Execute(script);
        
        // Get stack
        Value<Stack> stack = console.GetExecutor()->GetDataStack();
        cout << "Stack size after execution: " << stack->Size() << endl;
        
        // Print stack contents
        if (stack->Size() > 0) {
            cout << "Stack contents:" << endl;
            int index = 0;
            while (!stack->Empty()) {
                Object obj = stack->Pop();
                cout << "  [" << index++ << "]: " << obj.ToString() << endl;
            }
        }
        
        cout << "Test completed successfully!" << endl;
        return 0;
    }
    catch (Exception::Base &e) {
        cerr << "KAI Exception: " << e.ToString() << endl;
    }
    catch (exception &e) {
        cerr << "Standard Exception: " << e.what() << endl;
    }
    catch (...) {
        cerr << "Unknown exception" << endl;
    }
    
    return 1;
}
EOF
    
    # Try to compile it using g++
    echo "Compiling custom test program..."
    g++ -o test_dowhile "$TEST_CPP" -I./Include -L./Lib -lKAICore
    
    if [ $? -eq 0 ]; then
        echo "Running custom test program..."
        ./test_dowhile "$TEST_FILE"
    else
        echo "Compilation failed, trying to run the test with TestRho instead..."
        # Custom test for TestRho
        ./Bin/Test/TestRho --gtest_filter="TestRho.RunScripts"
    fi
else
    echo "ERROR: No Rho interpreter found"
    exit 1
fi

# Clean up
rm -f "$TEST_FILE"

echo "=== Test complete ==="
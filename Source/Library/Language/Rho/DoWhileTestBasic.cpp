#include <KAI/Core/Console.h>
#include <KAI/Executor/Executor.h>
#include <KAI/Language/Common/ProcessCommon.h>
#include <KAI/Language/Rho/RhoParser.h>
#include <KAI/Language/Rho/RhoTranslator.h>

#include <iostream>
#include <string>

using namespace KAI;
using namespace std;

int main() {
    try {
        // Create registry
        Registry reg;

        // Create a console (which handles input properly)
        Console console(reg);
        console.SetLanguage(Language::Rho);

        // Set trace level for detailed debugging
        console.GetExecutor()->SetTraceLevel(5);

        cout << "=== Testing do-while implementation ===" << endl;

        // Simple do-while script
        const char *script =
            "// Simple do-while test with proper indentation\n"
            "i = 0\n"
            "do\n"
            "    i = i + 1\n"
            "while i < 3\n"
            "i\n";

        cout << "Script:" << endl << script << endl;

        // Execute the script
        cout << "Executing script..." << endl;
        console.Execute(script);

        // Check result
        cout << "Checking results..." << endl;
        Value<Stack> stack = console.GetExecutor()->GetDataStack();
        cout << "Stack size: " << stack->Size() << endl;

        // Print the entire stack
        if (stack->Size() > 0) {
            cout << "Stack contents:" << endl;
            for (int i = 0; i < stack->Size(); i++) {
                Object obj = stack->At(i);
                if (obj.GetClass()) {
                    cout << "  [" << i << "]: " << obj.ToString()
                         << " (Type: " << obj.GetClass()->GetName() << ")"
                         << endl;
                } else {
                    cout << "  [" << i << "]: <no class>" << endl;
                }
            }
        }

        cout << "Test completed successfully!" << endl;
        return 0;
    } catch (Exception::Base &e) {
        cerr << "KAI Exception: " << e.ToString() << endl;
    } catch (exception &e) {
        cerr << "Standard Exception: " << e.what() << endl;
    } catch (...) {
        cerr << "Unknown exception" << endl;
    }

    return 1;
}
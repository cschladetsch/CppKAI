#include <iostream>
#include <memory>
#include <string>

#include "KAI/Core/Console.h"
#include "KAI/Core/File.h"
#include "KAI/KAI.h"
#include "KAI/Language/Rho/RhoTranslator.h"

using namespace kai;
using namespace std;

int main() {
    // Initialize console with high trace level
    Console console;
    console.SetLanguage(Language::Rho);
    console.GetExecutor()->SetTraceLevel(5);

    // Register built-in types
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<float>(Label("float"));
    reg.AddClass<String>(Label("String"));
    reg.AddClass<bool>(Label("bool"));
    reg.AddClass<Array>(Label("Array"));

    try {
        // Try a simple do-while loop
        const char* script =
            "i = 0\n"
            "do\n"
            "    i = i + 1\n"
            "while (i < 3)\n";

        cout << "Executing script:\n" << script << endl;
        console.Execute(script);

        // Print the value of i after execution
        int i = ConstDeref<int>(console.GetRegistry().GetGlobal(Label("i")));
        cout << "Final value of i: " << i << endl;

        cout << "Execution succeeded!" << endl;
    } catch (const Exception::Base& e) {
        cerr << "KAI Exception: " << e.ToString() << endl;
        return 1;
    } catch (const std::exception& e) {
        cerr << "Standard Exception: " << e.what() << endl;
        return 2;
    } catch (...) {
        cerr << "Unknown Exception" << endl;
        return 3;
    }

    return 0;
}
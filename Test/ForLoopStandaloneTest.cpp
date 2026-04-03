#include <KAI/Core/Console.h>

#include <iostream>

KAI_BEGIN

int main() {
    std::cout << "=== For Loop Standalone Test ===" << std::endl;

    // Create registry and console
    Registry reg;
    reg.AddClass<int>("int");
    reg.AddClass<bool>("bool");
    reg.AddClass<float>("float");
    reg.AddClass<String>("String");
    reg.AddClass<Array>("Array");
    reg.AddClass<Map>("Map");

    Console console(&reg);
    console.SetLanguage(Language::Rho);

    auto exec = console.GetExecutor();
    exec->SetTraceLevel(5);  // Maximum tracing for debugging

    try {
        std::cout << "Executing simple for loop..." << std::endl;

        // Execute a very simple for loop
        console.Execute(
            "sum = 0;\n"
            "for (i = 0; i < 5; i = i + 1) {\n"
            "    sum = sum + i;\n"
            "}\n");

        // Print result
        auto stack = exec->GetDataStack();
        std::cout << "Stack size after execution: " << stack->Size()
                  << std::endl;

        // Verify result by checking the value of sum
        console.Execute("sum");

        std::cout << "Final value of sum: " << exec->GetDataStack()->Top()
                  << std::endl;

        // Expected result: sum should be 0+1+2+3+4 = 10
        int sum = ConstDeref<int>(exec->GetDataStack()->Top());
        if (sum == 10) {
            std::cout << "✅ TEST PASSED: Sum is correctly calculated as 10"
                      << std::endl;
        } else {
            std::cout << "❌ TEST FAILED: Expected sum to be 10, but got "
                      << sum << std::endl;
        }

        // Try a more complex for loop with break
        std::cout << "\nExecuting for loop with break condition..."
                  << std::endl;

        console.Execute(
            "result = 0;\n"
            "for (i = 0; i < 10; i = i + 1) {\n"
            "    result = result + i;\n"
            "    if (result > 10) {\n"
            "        break;\n"
            "    }\n"
            "}\n");

        // Verify the result
        console.Execute("result");
        std::cout << "Final value of result: " << exec->GetDataStack()->Top()
                  << std::endl;

        // Expected result: result should be 0+1+2+3+4+5=15 (then breaks)
        int result = ConstDeref<int>(exec->GetDataStack()->Top());
        if (result == 15) {
            std::cout << "✅ TEST PASSED: Break condition works correctly"
                      << std::endl;
        } else {
            std::cout << "❌ TEST FAILED: Expected result to be 15, but got "
                      << result << std::endl;
        }

        // Try a nested for loop
        std::cout << "\nExecuting nested for loops..." << std::endl;

        console.Execute(
            "nested_sum = 0;\n"
            "for (i = 0; i < 3; i = i + 1) {\n"
            "    for (j = 0; j < 3; j = j + 1) {\n"
            "        nested_sum = nested_sum + (i * 3 + j);\n"
            "    }\n"
            "}\n");

        // Verify the result
        console.Execute("nested_sum");
        std::cout << "Final value of nested_sum: "
                  << exec->GetDataStack()->Top() << std::endl;

        // Expected result: Sum of 0 through 8 = 36
        int nested_sum = ConstDeref<int>(exec->GetDataStack()->Top());
        if (nested_sum == 36) {
            std::cout << "✅ TEST PASSED: Nested for loops work correctly"
                      << std::endl;
        } else {
            std::cout
                << "❌ TEST FAILED: Expected nested_sum to be 36, but got "
                << nested_sum << std::endl;
        }

        std::cout << "\nFor loop tests completed successfully!" << std::endl;
        return 0;
    } catch (const Exception::Base& e) {
        std::cerr << "KAI exception: " << e.ToString() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Standard exception: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Unknown exception" << std::endl;
    }

    return 1;
}

KAI_END
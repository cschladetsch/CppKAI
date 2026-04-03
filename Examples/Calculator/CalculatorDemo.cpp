// CalculatorDemo.cpp - Demonstrates distributed calculator using Tau-generated code
// Shows Future<int> usage pattern as requested

#include <iostream>
#include <cassert>
#include <memory>

// Note: This demonstrates the concept of Future<int> usage
// The actual generated proxy uses std::future internally

// Simplified Future<int> wrapper matching the requested API
template<typename T>
struct Future {
    std::future<T> impl_;

    Future(std::future<T>&& f) : impl_(std::move(f)) {}

    // Check if result is available (non-blocking)
    bool Available() const {
        return impl_.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready;
    }

    // Get the result (blocking if not available)
    T Result() {
        return impl_.get();
    }

    // Wait for completion
    void Wait() {
        impl_.wait();
    }
};

// Simple mock calculator proxy to demonstrate the pattern
// In real usage, this would be the generated ICalculatorProxy class
class CalculatorProxy {
public:
    // Async operation returning Future<int>
    Future<int> Add(int a, int b) {
        // In real implementation, this would send network request
        // and return a future that completes when response arrives
        auto promise = std::make_shared<std::promise<int>>();
        auto future = promise->get_future();

        // Simulate async calculation
        std::thread([promise, a, b]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            promise->set_value(a + b);
        }).detach();

        return Future<int>(std::move(future));
    }

    Future<int> Subtract(int a, int b) {
        auto promise = std::make_shared<std::promise<int>>();
        auto future = promise->get_future();

        std::thread([promise, a, b]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            promise->set_value(a - b);
        }).detach();

        return Future<int>(std::move(future));
    }

    Future<int> Multiply(int a, int b) {
        auto promise = std::make_shared<std::promise<int>>();
        auto future = promise->get_future();

        std::thread([promise, a, b]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            promise->set_value(a * b);
        }).detach();

        return Future<int>(std::move(future));
    }

    Future<int> Divide(int a, int b) {
        auto promise = std::make_shared<std::promise<int>>();
        auto future = promise->get_future();

        std::thread([promise, a, b]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            if (b == 0) {
                promise->set_exception(std::make_exception_ptr(
                    std::runtime_error("Division by zero")));
            } else {
                promise->set_value(a / b);
            }
        }).detach();

        return Future<int>(std::move(future));
    }
};

void DemonstrateBasicUsage() {
    std::cout << "=== Basic Calculator Demo ===" << std::endl << std::endl;

    CalculatorProxy proxy;

    // Example 1: Basic usage as requested
    std::cout << "Test 1: Future<int> f = proxy.Add(1, 2)" << std::endl;
    Future<int> f = proxy.Add(1, 2);

    // Wait for result
    f.Wait();

    std::cout << "  f.Available() = " << (f.Available() ? "true" : "false") << std::endl;
    std::cout << "  f.Result() = " << f.Result() << std::endl;
    assert(f.Available() && f.Result() == 3);
    std::cout << "  ✓ Assertion passed!" << std::endl << std::endl;

    // Example 2: Multiple concurrent operations
    std::cout << "Test 2: Multiple concurrent operations" << std::endl;
    auto f1 = proxy.Add(10, 20);
    auto f2 = proxy.Subtract(50, 15);
    auto f3 = proxy.Multiply(7, 6);

    std::cout << "  Waiting for results..." << std::endl;
    f1.Wait();
    f2.Wait();
    f3.Wait();

    std::cout << "  10 + 20 = " << f1.Result() << std::endl;
    std::cout << "  50 - 15 = " << f2.Result() << std::endl;
    std::cout << "  7 * 6 = " << f3.Result() << std::endl;
    assert(f1.Result() == 30);
    assert(f2.Result() == 35);
    assert(f3.Result() == 42);
    std::cout << "  ✓ All assertions passed!" << std::endl << std::endl;
}

void DemonstrateInfixEvaluation() {
    std::cout << "=== Infix Expression Evaluation ===" << std::endl << std::endl;

    CalculatorProxy proxy;

    // Simulate: (5 + 3) * 2
    std::cout << "Evaluating: (5 + 3) * 2" << std::endl;
    auto step1 = proxy.Add(5, 3);
    step1.Wait();
    int intermediate = step1.Result();
    std::cout << "  Step 1: 5 + 3 = " << intermediate << std::endl;

    auto step2 = proxy.Multiply(intermediate, 2);
    step2.Wait();
    int result = step2.Result();
    std::cout << "  Step 2: " << intermediate << " * 2 = " << result << std::endl;
    std::cout << "  Final result: " << result << std::endl;
    assert(result == 16);
    std::cout << "  ✓ Assertion passed!" << std::endl << std::endl;
}

void DemonstrateErrorHandling() {
    std::cout << "=== Error Handling Demo ===" << std::endl << std::endl;

    CalculatorProxy proxy;

    std::cout << "Test: Division by zero" << std::endl;
    try {
        auto f = proxy.Divide(10, 0);
        f.Wait();
        int result = f.Result();  // This will throw
        std::cout << "  ERROR: Should have thrown!" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "  ✓ Caught exception: " << e.what() << std::endl;
    }
    std::cout << std::endl;
}

void DemonstrateNonBlockingCheck() {
    std::cout << "=== Non-Blocking Availability Check ===" << std::endl << std::endl;

    CalculatorProxy proxy;

    std::cout << "Starting async operation: Add(100, 200)" << std::endl;
    auto f = proxy.Add(100, 200);

    // Poll for availability
    int attempts = 0;
    while (!f.Available() && attempts++ < 5) {
        std::cout << "  Not ready yet... (attempt " << attempts << ")" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    if (f.Available()) {
        std::cout << "  Result is ready!" << std::endl;
        std::cout << "  Result = " << f.Result() << std::endl;
        assert(f.Result() == 300);
        std::cout << "  ✓ Assertion passed!" << std::endl;
    } else {
        std::cout << "  Waiting for completion..." << std::endl;
        f.Wait();
        std::cout << "  Result = " << f.Result() << std::endl;
    }
    std::cout << std::endl;
}

int main() {
    std::cout << "Distributed Calculator Demo" << std::endl;
    std::cout << "Using Tau-generated IDL code pattern" << std::endl;
    std::cout << "========================================" << std::endl << std::endl;

    try {
        DemonstrateBasicUsage();
        DemonstrateInfixEvaluation();
        DemonstrateErrorHandling();
        DemonstrateNonBlockingCheck();

        std::cout << "========================================" << std::endl;
        std::cout << "All tests passed!" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

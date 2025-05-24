#include <gtest/gtest.h>

#include <iostream>
#include <string>

#include "../TestLangCommon.h"
#include "KAI/Language/Rho/RhoParser.h"

using namespace kai;
using namespace std;

// Test fixture for additional Rho tests
class NewRhoTests : public TestLangCommon {
   protected:
    void SetUp() override {
        TestLangCommon::SetUp();
        // Set up for Rho language
        console_.SetLanguage(Language::Rho);
    }
};

// Simple recursive factorial function
TEST_F(NewRhoTests, FactorialRecursive) {
    // Expected factorial of 5 is 120
    const int expected = 120;

    // Execute factorial function
    console_.Execute(R"(
        function factorial(n) {
            if (n <= 1) return 1;
            return n * factorial(n - 1);
        }
        factorial(5);
    )");

    // Verify result
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(data_->Top()), expected);
}

// Fibonacci with loop
TEST_F(NewRhoTests, FibonacciLoop) {
    // Expected Fibonacci number at position 10 is 55
    const int expected = 55;

    // Execute Fibonacci function
    console_.Execute(R"(
        function fibonacci(n) {
            if (n <= 1) return n;
            
            var a = 0;
            var b = 1;
            var result = 0;
            
            for (var i = 2; i <= n; i = i + 1) {
                result = a + b;
                a = b;
                b = result;
            }
            
            return result;
        }
        fibonacci(10);
    )");

    // Verify result
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(data_->Top()), expected);
}

// GCD implementation
TEST_F(NewRhoTests, GCD) {
    // Expected GCD of 48 and 18 is 6
    const int expected = 6;

    // Execute GCD function
    console_.Execute(R"(
        function gcd(a, b) {
            while (b != 0) {
                var temp = b;
                b = a % b;
                a = temp;
            }
            return a;
        }
        gcd(48, 18);
    )");

    // Verify result
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(data_->Top()), expected);
}

// String concatenation
TEST_F(NewRhoTests, StringConcatenation) {
    // Execute string manipulation function
    console_.Execute(R"(
        function testString() {
            var a = "Hello";
            var b = " World";
            var c = a + b;
            if (c == "Hello World") return 1;
            return 0;
        }
        testString();
    )");

    // Verify result
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 1);
}

// Array operations
TEST_F(NewRhoTests, ArrayOperations) {
    // Expected sum is 15 (1+2+3+4+5)
    const int expected = 15;

    // Execute array operations
    console_.Execute(R"(
        var arr = [1, 2, 3, 4, 5];
        var sum = 0;
        
        for (var i = 0; i < arr.length(); i = i + 1) {
            sum = sum + arr[i];
        }
        
        sum;
    )");

    // Verify result
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(data_->Top()), expected);
}

// Function with default parameters
TEST_F(NewRhoTests, DefaultParameters) {
    // Expected sum is 22 (7+5+10)
    const int expected = 22;

    // Execute function with default parameters
    console_.Execute(R"(
        function addValues(a, b, c) {
            if (b == undefined) b = 5;
            if (c == undefined) c = 10;
            return a + b + c;
        }
        
        addValues(7);
    )");

    // Verify result
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(data_->Top()), expected);
}

// Function composition
TEST_F(NewRhoTests, FunctionComposition) {
    // Expected result is 12 (2*(5+1))
    const int expected = 12;

    // Execute function composition
    console_.Execute(R"(
        function double(x) { return x * 2; }
        function increment(x) { return x + 1; }
        
        function compose(f, g) {
            return function(x) {
                return f(g(x));
            };
        }
        
        var incrementThenDouble = compose(double, increment);
        incrementThenDouble(5);
    )");

    // Verify result
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(data_->Top()), expected);
}

// Nested loops
TEST_F(NewRhoTests, NestedLoops) {
    // Execute nested loops
    console_.Execute(R"(
        var result = 0;
        for (var i = 0; i < 5; i = i + 1) {
            for (var j = 0; j < 5; j = j + 1) {
                if (i == j) {
                    result = result + 1;
                }
            }
        }
        result;
    )");

    // Verify result - should be 5 diagonal matches
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 5);
}

// Object properties
TEST_F(NewRhoTests, ObjectProperties) {
    // Execute object property access
    console_.Execute(R"(
        var person = {
            name: "John",
            age: 30,
            address: {
                city: "New York",
                zip: 10001
            }
        };
        
        person.address.zip;
    )");

    // Verify result
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 10001);
}

// Conditional logic
TEST_F(NewRhoTests, ConditionalLogic) {
    // Execute conditional logic
    console_.Execute(R"(
        var x = 10;
        var result = 0;
        
        if (x > 5) {
            result = 1;
        } else if (x < 5) {
            result = -1;
        } else {
            result = 0;
        }
        
        result;
    )");

    // Verify result
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 1);
}

// Array filtering functionality
TEST_F(NewRhoTests, ArrayFiltering) {
    // Execute array filtering
    console_.Execute(R"(
        var numbers = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10];
        var evens = [];
        
        for (var i = 0; i < numbers.length(); i = i + 1) {
            if (numbers[i] % 2 == 0) {
                evens.push(numbers[i]);
            }
        }
        
        evens.length();
    )");

    // Verify result - should be 5 even numbers
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 5);
}

// Recursive power function
TEST_F(NewRhoTests, PowerFunction) {
    // Execute power function
    console_.Execute(R"(
        function power(base, exponent) {
            if (exponent == 0) return 1;
            return base * power(base, exponent - 1);
        }
        
        power(2, 8);
    )");

    // Verify result - 2^8 = 256
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 256);
}

// While loop with break
TEST_F(NewRhoTests, WhileLoopWithBreak) {
    // Execute while loop with break
    console_.Execute(R"(
        var i = 0;
        var sum = 0;
        
        while (true) {
            i = i + 1;
            sum = sum + i;
            
            if (i >= 10) break;
        }
        
        sum;
    )");

    // Verify result - sum of 1 to 10 is 55
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 55);
}

// For loop with continue
TEST_F(NewRhoTests, ForLoopWithContinue) {
    // Execute for loop with continue
    console_.Execute(R"(
        var sum = 0;
        
        for (var i = 1; i <= 10; i = i + 1) {
            // Skip odd numbers
            if (i % 2 != 0) continue;
            
            sum = sum + i;
        }
        
        sum;
    )");

    // Verify result - sum of even numbers from 1 to 10 is 30 (2+4+6+8+10)
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 30);
}

// Bubble sort implementation
TEST_F(NewRhoTests, BubbleSort) {
    // Execute bubble sort
    console_.Execute(R"(
        function bubbleSort(arr) {
            var n = arr.length();
            
            for (var i = 0; i < n; i = i + 1) {
                for (var j = 0; j < n - i - 1; j = j + 1) {
                    if (arr[j] > arr[j + 1]) {
                        var temp = arr[j];
                        arr[j] = arr[j + 1];
                        arr[j + 1] = temp;
                    }
                }
            }
            
            return arr;
        }
        
        var arr = [5, 3, 8, 4, 2];
        var sorted = bubbleSort(arr);
        sorted[2]; // Middle element should be 4
    )");

    // Verify result
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 4);
}

// Callback pattern
TEST_F(NewRhoTests, CallbackPattern) {
    // Execute callback pattern
    console_.Execute(R"(
        function processData(data, callback) {
            var transformed = data * 2;
            return callback(transformed);
        }
        
        function addFive(val) {
            return val + 5;
        }
        
        processData(10, addFive);
    )");

    // Verify result - (10*2)+5 = 25
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 25);
}

// Recursive calculation
TEST_F(NewRhoTests, RecursiveCalculation) {
    // Execute recursive sum calculation
    console_.Execute(R"(
        function sumToN(n) {
            if (n <= 1) return n;
            return n + sumToN(n - 1);
        }
        
        sumToN(10);
    )");

    // Verify result - sum from 1 to 10 is 55
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 55);
}

// Ternary-like operation
TEST_F(NewRhoTests, TernaryOperation) {
    // Execute ternary-like operation using if/else
    console_.Execute(R"(
        function ternary(condition, trueValue, falseValue) {
            if (condition) return trueValue;
            return falseValue;
        }
        
        ternary(5 > 3, 10, 20);
    )");

    // Verify result - condition is true, so result should be 10
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 10);
}

// Array transformation
TEST_F(NewRhoTests, ArrayTransformation) {
    // Execute array transformation (square each element)
    console_.Execute(R"(
        var numbers = [1, 2, 3, 4, 5];
        var squares = [];
        
        for (var i = 0; i < numbers.length(); i = i + 1) {
            squares.push(numbers[i] * numbers[i]);
        }
        
        // Sum of squares: 1+4+9+16+25 = 55
        var sum = 0;
        for (var i = 0; i < squares.length(); i = i + 1) {
            sum = sum + squares[i];
        }
        
        sum;
    )");

    // Verify result - sum of squares from 1 to 5 is 55
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 55);
}

// Prime number checker
TEST_F(NewRhoTests, PrimeNumberChecker) {
    // Execute prime number counter
    console_.Execute(R"(
        function isPrime(num) {
            if (num <= 1) return false;
            if (num <= 3) return true;
            
            if (num % 2 == 0 || num % 3 == 0) return false;
            
            for (var i = 5; i * i <= num; i = i + 6) {
                if (num % i == 0 || num % (i + 2) == 0) return false;
            }
            
            return true;
        }
        
        function countPrimesUpTo(n) {
            var count = 0;
            for (var i = 2; i <= n; i = i + 1) {
                if (isPrime(i)) count = count + 1;
            }
            return count;
        }
        
        countPrimesUpTo(20);
    )");

    // Verify result - 8 primes up to 20: 2, 3, 5, 7, 11, 13, 17, 19
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 8);
}
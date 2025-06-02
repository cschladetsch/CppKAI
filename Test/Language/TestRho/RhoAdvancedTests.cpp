#include <gtest/gtest.h>
#include <vector>
#include <map>
#include <algorithm>

#include "TestLangCommon.h"

using namespace kai;
using namespace std;

// Advanced Rho test fixture
struct RhoAdvancedTests : TestLangCommon {
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(Language::Rho);
    }
    
    template <typename T>
    void RunAndExpect(const string& code, T expected) {
        console_.Execute(code, Structure::Program);
        auto stack = console_.GetExecutor()->GetDataStack();
        ASSERT_FALSE(stack->Empty()) << "No result on stack after: " << code;
        ASSERT_EQ(ConstDeref<T>(stack->Top()), expected) << "For code: " << code;
    }
};

// Test 1: Complex expression evaluation with precedence
TEST_F(RhoAdvancedTests, ComplexExpressionPrecedence) {
    RunAndExpect<int>(R"(
a = 10
b = 5
c = 2
result = a + b * c - a / b + (a % b) * c
result
)", 18); // 10 + 5*2 - 10/2 + (10%5)*2 = 10 + 10 - 2 + 0 = 18
}

// Test 2: Function composition and higher-order functions
TEST_F(RhoAdvancedTests, FunctionComposition) {
    RunAndExpect<int>(R"(
fun double = x
	return x * 2

fun addOne = x
	return x + 1

fun compose = f, g
	fun composed = x
		return f(g(x))
	return composed

doubleAddOne = compose(double, addOne)
result = doubleAddOne(5)
result
)", 12); // (5 + 1) * 2 = 12
}

// Test 3: Recursive data structure manipulation
TEST_F(RhoAdvancedTests, RecursiveDataStructures) {
    RunAndExpect<int>(R"(
fun sumTree = node {
    if node == null {
        return 0
    }
    return node.value + sumTree(node.left) + sumTree(node.right)
}

// Build a simple tree manually
root = { value: 10 }
root.left = { value: 5 }
root.right = { value: 15 }
root.left.left = { value: 3 }
root.left.right = { value: 7 }

total = sumTree(root)
total
)", 40); // 10 + 5 + 15 + 3 + 7 = 40
}

// Test 4: List comprehension simulation
TEST_F(RhoAdvancedTests, ListComprehension) {
    RunAndExpect<int>(R"(
fun map = list, fn {
    result = []
    i = 0
    while i < list.length {
        result[i] = fn(list[i])
        i = i + 1
    }
    return result
}

fun filter = list, pred {
    result = []
    j = 0
    for i = 0; i < list.length; i = i + 1 {
        if pred(list[i]) {
            result[j] = list[i]
            j = j + 1
        }
    }
    return result
}

numbers = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]

// Square all even numbers
fun isEven = x { return x % 2 == 0 }
fun square = x { return x * x }
evens = filter(numbers, isEven)
squares = map(evens, square)

// Sum the results
sum = 0
for i = 0; i < squares.length; i = i + 1 {
    sum = sum + squares[i]
}
sum
)", 220); // 4 + 16 + 36 + 64 + 100 = 220
}

// Test 5: Closure and lexical scoping
TEST_F(RhoAdvancedTests, ClosuresAndScoping) {
    RunAndExpect<int>(R"(
fun makeCounter = start {
    count = start
    
    fun increment = {
        count = count + 1
        return count
    }
    
    fun decrement = {
        count = count - 1
        return count
    }
    
    fun get = {
        return count
    }
    
    return { inc: increment, dec: decrement, val: get }
}

counter = makeCounter(10)
counter.inc()
counter.inc()
counter.dec()
result = counter.val()
result
)", 11);
}

// Test 6: Exception handling simulation
TEST_F(RhoAdvancedTests, ExceptionHandling) {
    RunAndExpect<int>(R"(
fun safeDivide = a, b {
    if b == 0 {
        return { error: true, message: "Division by zero" }
    }
    return { error: false, value: a / b }
}

fun calculate = x, y {
    result = safeDivide(x, y)
    if result.error {
        return -1
    }
    return result.value * 100
}

// Test normal case
val1 = calculate(10, 2)

// Test error case
val2 = calculate(10, 0)

val1 + val2
)", 499); // 500 + (-1) = 499
}

// Test 7: Dynamic dispatch simulation
TEST_F(RhoAdvancedTests, DynamicDispatch) {
    RunAndExpect<int>(R"(
fun makeShape = type {
    if type == "circle" {
        fun circleArea = self { return 3 * self.radius * self.radius }
        return {
            type: "circle",
            radius: 5,
            area: circleArea
        }
    }
    if type == "square" {
        fun squareArea = self { return self.side * self.side }
        return {
            type: "square",
            side: 4,
            area: squareArea
        }
    }
}

shapes = []
shapes[0] = makeShape("circle")
shapes[1] = makeShape("square")

totalArea = 0
for i = 0; i < 2; i = i + 1 {
    totalArea = totalArea + shapes[i].area(shapes[i])
}
totalArea
)", 91); // 3*5*5 + 4*4 = 75 + 16 = 91
}

// Test 8: Memoization implementation
TEST_F(RhoAdvancedTests, Memoization) {
    RunAndExpect<int>(R"(
memo = {}

fun fib = n {
    key = "fib_" + n
    if memo[key] != null {
        return memo[key]
    }
    
    if n <= 1 {
        result = n
    } else {
        result = fib(n - 1) + fib(n - 2)
    }
    
    memo[key] = result
    return result
}

// Calculate 10th Fibonacci number
result = fib(10)
result
)", 55);
}

// Test 9: Pattern matching simulation
TEST_F(RhoAdvancedTests, PatternMatching) {
    RunAndExpect<String>(R"(
fun match = value, patterns {
    for i = 0; i < patterns.length; i = i + 1 {
        pattern = patterns[i]
        if pattern.pred(value) {
            return pattern.action(value)
        }
    }
    return "no match"
}

fun isNegative = x { return x < 0 }
fun isZero = x { return x == 0 }
fun isEven = x { return x % 2 == 0 }
fun isTrue = x { return true }

fun returnNegative = x { return "negative" }
fun returnZero = x { return "zero" }
fun returnEven = x { return "even" }
fun returnOdd = x { return "odd" }

result = match(42, [
    { pred: isNegative, action: returnNegative },
    { pred: isZero, action: returnZero },
    { pred: isEven, action: returnEven },
    { pred: isTrue, action: returnOdd }
])
result
)", "even");
}

// Test 10: Lazy evaluation simulation
TEST_F(RhoAdvancedTests, LazyEvaluation) {
    RunAndExpect<int>(R"(
fun lazy = fn {
    evaluated = false
    value = null
    
    fun lazyEval = {
        if !evaluated {
            value = fn()
            evaluated = true
        }
        return value
    }
    return lazyEval
}

fun expensive = {
    sum = 0
    for i = 1; i <= 100; i = i + 1 {
        sum = sum + i
    }
    return sum
}

expensiveCalc = lazy(expensive)

// First call does the calculation
result1 = expensiveCalc()

// Second call returns cached value
result2 = expensiveCalc()

result1 + result2
)", 10100); // 5050 + 5050
}

// Test 11: Pipeline operator simulation
TEST_F(RhoAdvancedTests, PipelineOperator) {
    RunAndExpect<int>(R"(
fun pipe = value, functions {
    result = value
    for i = 0; i < functions.length; i = i + 1 {
        result = functions[i](result)
    }
    return result
}

fun add3 = x { return x + 3 }
fun times2 = x { return x * 2 }
fun minus4 = x { return x - 4 }
fun squared = x { return x * x }

result = pipe(5, [add3, times2, minus4, squared])
result
)", 144); // ((5 + 3) * 2 - 4)^2 = 12^2 = 144
}

// Test 12: Coroutine simulation
TEST_F(RhoAdvancedTests, CoroutineSimulation) {
    RunAndExpect<int>(R"(
fun makeGenerator = start, end {
    current = start
    
    fun next = {
        if current <= end {
            val = current
            current = current + 1
            return { done: false, value: val }
        } else {
            return { done: true, value: null }
        }
    }
    return next
}

gen = makeGenerator(1, 5)
sum = 0

cont = true
while cont {
    result = gen()
    if result.done {
        cont = false
    } else {
        sum = sum + result.value
    }
}
sum
)", 15); // 1 + 2 + 3 + 4 + 5
}

// Test 13: Complex control flow with breaks
TEST_F(RhoAdvancedTests, ComplexControlFlow) {
    RunAndExpect<int>(R"(
result = 0
for i = 1; i <= 10; i = i + 1 {
    if i % 2 == 0 {
        continue
    }
    
    for j = 1; j <= i; j = j + 1 {
        result = result + j
        if j >= 3 {
            break
        }
    }
    
    if i >= 7 {
        break
    }
}
result
)", 18); // Complex nested loop calculation
}

// Test 14: Object-oriented programming patterns
TEST_F(RhoAdvancedTests, ObjectOrientedPatterns) {
    RunAndExpect<int>(R"(
fun makeClass = constructor {
    prototype = {}
    
    fun newInstance = x, y {
        instance = { __proto__: prototype }
        constructor(instance, x, y)
        return instance
    }
    
    return { new: newInstance, prototype: prototype }
}

fun pointConstructor = self, x, y {
    self.x = x
    self.y = y
}

Point = makeClass(pointConstructor)

fun distanceMethod = self, other {
    dx = self.x - other.x
    dy = self.y - other.y
    return dx * dx + dy * dy  // squared distance
}

Point.prototype.distance = distanceMethod

p1 = Point.new(3, 4)
p2 = Point.new(0, 0)

dist = p1.distance(p1, p2)
dist
)", 25); // 3^2 + 4^2 = 25
}

// Test 15: Functional programming utilities
TEST_F(RhoAdvancedTests, FunctionalUtilities) {
    RunAndExpect<int>(R"(
fun reduce = arr, fn, initial {
    acc = initial
    for i = 0; i < arr.length; i = i + 1 {
        acc = fn(acc, arr[i])
    }
    return acc
}

fun partial = fn, arg1 {
    fun partialApplied = arg2 {
        return fn(arg1, arg2)
    }
    return partialApplied
}

fun add = a, b { return a + b }
add5 = partial(add, 5)

numbers = [1, 2, 3, 4]
sum = reduce(numbers, add, 0)

result = add5(sum)
result
)", 15); // sum=10, add5(10)=15
}

// Test 16: State machine implementation
TEST_F(RhoAdvancedTests, StateMachine) {
    RunAndExpect<String>(R"(
fun makeStateMachine = {
    state = "idle"
    
    transitions = {
        idle: { start: "running" },
        running: { pause: "paused", stop: "idle" },
        paused: { resume: "running", stop: "idle" }
    }
    
    fun getState = { return state }
    fun transition = action {
        if transitions[state][action] {
            state = transitions[state][action]
            return true
        }
        return false
    }
    
    return {
        getState: getState,
        transition: transition
    }
}

machine = makeStateMachine()
machine.transition("start")
machine.transition("pause")
result = machine.getState()
result
)", "paused");
}

// Test 17: Matrix operations
TEST_F(RhoAdvancedTests, MatrixOperations) {
    RunAndExpect<int>(R"(
fun matrixMultiply = a, b {
    rows = a.length
    cols = b[0].length
    n = b.length
    
    result = []
    for i = 0; i < rows; i = i + 1 {
        result[i] = []
        for j = 0; j < cols; j = j + 1 {
            sum = 0
            for k = 0; k < n; k = k + 1 {
                sum = sum + a[i][k] * b[k][j]
            }
            result[i][j] = sum
        }
    }
    return result
}

m1 = [[1, 2], [3, 4]]
m2 = [[5, 6], [7, 8]]

result = matrixMultiply(m1, m2)
result[1][1]
)", 53); // [3,4] * [[5,6],[7,8]] => result[1][1] = 3*7 + 4*8 = 21 + 32 = 53
}

// Test 18: Event system
TEST_F(RhoAdvancedTests, EventSystem) {
    RunAndExpect<int>(R"(
fun makeEventEmitter = {
    listeners = {}
    
    fun on = event, callback {
        if !listeners[event] {
            listeners[event] = []
        }
        listeners[event][listeners[event].length] = callback
    }
    
    fun emit = event, data {
        if listeners[event] {
            for i = 0; i < listeners[event].length; i = i + 1 {
                listeners[event][i](data)
            }
        }
    }
    
    return {
        on: on,
        emit: emit
    }
}

emitter = makeEventEmitter()
sum = 0

fun addValue = value { sum = sum + value }
fun addDouble = value { sum = sum + value * 2 }

emitter.on("add", addValue)
emitter.on("add", addDouble)

emitter.emit("add", 10)
sum
)", 30); // 10 + 10*2 = 30
}

// Test 19: Advanced string processing
TEST_F(RhoAdvancedTests, AdvancedStringProcessing) {
    RunAndExpect<String>(R"(
fun split = str, delimiter {
    result = []
    current = ""
    count = 0
    
    for i = 0; i < str.length; i = i + 1 {
        if str[i] == delimiter {
            result[count] = current
            current = ""
            count = count + 1
        } else {
            current = current + str[i]
        }
    }
    
    if current != "" {
        result[count] = current
    }
    
    return result
}

fun join = arr, delimiter {
    result = ""
    for i = 0; i < arr.length; i = i + 1 {
        if i > 0 {
            result = result + delimiter
        }
        result = result + arr[i]
    }
    return result
}

text = "hello,world,test"
parts = split(text, ",")
result = join(parts, "-")
result
)", "hello-world-test");
}

// Test 20: Complex algorithm - quicksort
TEST_F(RhoAdvancedTests, QuickSortAlgorithm) {
    RunAndExpect<int>(R"(
fun quicksort = arr, low, high {
    if low < high {
        pi = partition(arr, low, high)
        quicksort(arr, low, pi - 1)
        quicksort(arr, pi + 1, high)
    }
}

fun partition = arr, low, high {
    pivot = arr[high]
    i = low - 1
    
    for j = low; j < high; j = j + 1 {
        if arr[j] < pivot {
            i = i + 1
            temp = arr[i]
            arr[i] = arr[j]
            arr[j] = temp
        }
    }
    
    temp = arr[i + 1]
    arr[i + 1] = arr[high]
    arr[high] = temp
    
    return i + 1
}

numbers = [64, 34, 25, 12, 22, 11, 90]
quicksort(numbers, 0, 6)

// Sum to verify sort worked
sum = 0
for i = 0; i < 7; i = i + 1 {
    sum = sum + numbers[i] * (i + 1)
}
sum
)", 904); // Weighted sum after sorting
}
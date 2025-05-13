# KAI Languages

The KAI system comes with a generic language-construction system that resides [here](Include/KAI/Language/Common). For comprehensive guides on each language, see the tutorials linked below.

## Pi

[Pi](Include/KAI/Language/Pi) is a post-fix language inspired by the amazing [HP48](https://en.wikipedia.org/wiki/HP_48_series) calculator and [Forth](https://en.wikipedia.org/wiki/Forth_\(programming_language\)).

Here's some sample Pi script:

```
{ 1 + } 'add # 2 add & print &
```

This creates a function named 'add', invokes it with an argument of 2, and prints the result.

The syntax uses reverse-polish notation: arguments are pushed onto the stack first, then operators are applied.

In this case:
1. The function **{ 1 + }** adds 1 to whatever is on the stack
2. **'add #** stores that function with the name "add" in the current scope
3. **2 add @** pushes 2 and the add function onto the stack
4. **&** executes the function (add) that's on the stack
5. **print &** pushes the print function onto the stack and executes it

The result is an empty stack with the console output of "3".

For more details and examples, see the [Pi Language Tutorial](Doc/PiTutorial.md).

## Rho

[Rho](Include/KAI/Language/Rho) is an *infix* language that translates to Pi code. It has a syntax similar to Python but with native continuations and the ability to embed Pi code directly.

Sample Rho code:

```rho
fun add(a) {
    return a + 1
}
print(add(2))
```

This example is functionally equivalent to the Pi example above. It defines a function that takes one argument, adds 1 to it, and returns the result. Then it calls the function with 2 and prints the result.

Rho gets translated to Pi code on the fly, making it both expressive and efficient.

For more details and examples, see the [Rho Language Tutorial](Doc/RhoTutorial.md).

## Tau

[Tau](Include/KAI/Language/Tau) is KAI's Interface Definition Language (IDL) used to describe objects that are visible across a network. 

Tau solves the problem of seamless network communication by:
1. Defining interfaces between distributed components
2. Generating proxy and agent code for transparent network communication
3. Ensuring type safety across network boundaries
4. Supporting versioning for backward compatibility

For example, a Tau file (foo.tau) generates:
- foo.agent.cpp/h files for hosting a service
- foo.proxy.cpp/h files for using a service from elsewhere

If you want to host a service, you implement what's required in the agent files. If you want to use a service from elsewhere, you simply use `kai::Proxy<Service>`.

For more details and examples, see the [Tau Language Tutorial](Doc/TauTutorial.md).

## Continuations

Continuations (also known as coroutines or fibers) are natively supported in both Pi and Rho languages.

Continuations allow you to yield the current command sequence to another without using threads. They are functions that can be interrupted at a certain point, then later resumed from that point.

What makes Pi and Rho unique is that they expose **two stacks**:
1. **Data stack**: Holds values being operated on
2. **Context stack**: Controls program flow

While these stacks exist in other language implementations, Pi and Rho allow you to directly manipulate the context stack, enabling powerful control flow patterns not easily accessible in most languages.

### Example in Pi
```pi
{ 
  "Step 1" trace
  yield
  "Step 2" trace
} 'process #

// Execute first part
process @
&

// Do something else
"Doing other work" trace

// Resume the continuation
continue
```

### Example in Rho
```rho
fun process() {
    print("Step 1")
    yield
    print("Step 2")
}

// Start the process
co = process()

// Do something else
print("Doing other work")

// Resume the continuation
co.resume()
```

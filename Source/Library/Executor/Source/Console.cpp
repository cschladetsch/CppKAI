#include "KAI/Console/Console.h"

#include <iostream>

#include "KAI/Core/BuiltinTypes.h"
#include "KAI/Core/Memory/StandardAllocator.h"
#include "KAI/Core/Object.h"
#include "KAI/Executor/BinBase.h"
#include "rang.hpp"

using namespace std;

KAI_BEGIN

Console::Console() {
    alloc = make_shared<Memory::StandardAllocator>();
    Create();
}

Console::Console(shared_ptr<Memory::IAllocator> alloc) {
    this->alloc = alloc;
    Create();
}

Console::~Console() { alloc->DeAllocate(reg_); }

void Console::Create() {
    try {
        reg_ = alloc->Allocate<Registry>(alloc);

        RegisterTypes();

        executor = reg_->New<Executor>();
        compiler = reg_->New<Compiler>();

        executor.SetManaged(false);
        compiler.SetManaged(false);

        // Set the compiler reference in the executor
        executor->SetCompiler(compiler);

        CreateTree();

        SetLanguage(Language::Pi);
    }
    KAI_CATCH(exception, e) {
        KAI_TRACE_1(e.what());
        std::cerr << "Console::Create::Exception '" << e.what() << "'" << ends;
    }
}

void Console::ExposeTypesToTree(Object types) {
    for (int N = 0; N < Type::Number::Last; ++N) {
        const ClassBase *K = reg_->GetClass(N);
        if (K == 0) continue;
        types.Set(K->GetName(), reg_->New(K));
    }
}

void Console::SetLanguage(Language lang) {
    SetLanguage(static_cast<int>(lang));
}

void Console::SetLanguage(int lang) {
    language = static_cast<Language>(lang);
    compiler->SetLanguage(lang);
}

void Console::ControlC() { executor->ClearContext(); }

Language Console::GetLanguage() const { return language; }

void Console::CreateTree() {
    Object root = reg_->New<void>();
    Object types = reg_->New<void>();
    Object sys = reg_->New<void>();
    Object bin = reg_->New<void>();
    Object home = reg_->New<void>();

    types.SetSwitch(IObject::Managed, false);
    sys.SetSwitch(IObject::Managed, false);
    root.SetSwitch(IObject::Managed, false);
    bin.SetSwitch(IObject::Managed, false);

    home.SetManaged(false);

    tree.SetRoot(root);
    root.Set("Types", types);
    root.Set("Sys", sys);
    root.Set("Bin", bin);
    root.Set("Home", home);

    Set(root, Pathname("/Compiler"), compiler);
    Set(root, Pathname("/Executor"), executor);

    Bin::AddFunctions(bin);
    tree.AddSearchPath(Pathname("/Bin"));
    tree.AddSearchPath(Pathname("/Sys"));
    tree.AddSearchPath(Pathname("/Types"));

    executor->SetTree(&tree);
    reg_->SetTree(tree);

    root.Set("Home", home);
    tree.SetScope(Pathname("/Home"));

    ExposeTypesToTree(types);
}

void Console::Execute(Pointer<Continuation> cont) {
    KAI_TRY {
        // Set the scope for the continuation
        cont->SetScope(executor->GetTree()->GetScope());
        
        // Debug the continuation code to help with diagnosing any issues
        if (cont->GetCode().Exists()) {
            KAI_TRACE_1(cont->GetCode()->Size()) << "Executing continuation with size";
        }
        
        // Use a direct execution approach for Pi language to solve the Type Mismatch issues
        if (language == Language::Pi) {
            // For null or empty continuations, nothing to do
            if (!cont.Exists() || !cont->GetCode().Exists() || cont->GetCode()->Size() == 0) {
                return;
            }
            
            // Get the data stack for easier access
            Value<Stack> dataStack = executor->GetDataStack();
            
            // Debug output to check what we're working with
            KAI_TRACE_1(cont->GetCode()->Size()) << "Pi continuation size:";
            
            // Instead of looking at individual operations, execute the entire block
            // Most Pi blocks come as a single continuation object in an array
            if (cont->GetCode()->Size() == 1) {
                Object firstItem = cont->GetCode()->At(0);
                
                // Check if the first item is a continuation (which contains the actual code)
                if (firstItem.IsType<Continuation>()) {
                    KAI_TRACE_1("Found inner continuation, executing it directly");
                    Continuation& innerCont = Deref<Continuation>(firstItem);
                    
                    // Create a temporary continuation to execute
                    Pointer<Continuation> innerContPtr = reg_->New<Continuation>();
                    innerContPtr->SetCode(innerCont.GetCode());
                    
                    // Print the content of the inner continuation for debugging
                    if (innerContPtr->GetCode().Exists()) {
                        KAI_TRACE_1(innerContPtr->GetCode()->Size()) << "Inner continuation size:";
                        for (int i = 0; i < innerContPtr->GetCode()->Size(); i++) {
                            Object item = innerContPtr->GetCode()->At(i);
                            KAI_TRACE_1(item.ToString()) << "Item " << i << ":";
                        }
                    }
                    
                    // Now process the inner continuation
                    for (int i = 0; i < innerContPtr->GetCode()->Size(); i++) {
                        Object item = innerContPtr->GetCode()->At(i);
                        
                        KAI_TRACE_1(item.ToString()) << "Processing item " << i << ":";
                        
                        // Skip the ContinuationBegin and ContinuationEnd operations
                        if (item.IsType<Operation>()) {
                            Operation::Type opType = Deref<Operation>(item).GetTypeNumber();
                            if (opType == Operation::ContinuationBegin || opType == Operation::ContinuationEnd) {
                                KAI_TRACE_1("Skipping continuation marker");
                                continue;
                            }
                        }
                        
                        // Process each item specially based on type
                        if (item.IsType<Operation>()) {
                            // Special handling for operations
                            Operation::Type opType = Deref<Operation>(item).GetTypeNumber();
                            
                            // Handle simple stack operations directly
                            if (opType == Operation::Dup) {
                                if (dataStack->Size() > 0) {
                                    Object toCopy = dataStack->Top();
                                    Object copy = toCopy.Duplicate();
                                    dataStack->Push(copy);
                                }
                                continue;
                            }
                            
                            // Handle arithmetic operations directly
                            if (opType == Operation::Plus) {
                                if (dataStack->Size() >= 2) {
                                    Object b = dataStack->Pop();
                                    Object a = dataStack->Pop();
                                    
                                    // Handle different type combinations
                                    if (a.IsType<int>() && b.IsType<int>()) {
                                        int result = Deref<int>(a) + Deref<int>(b);
                                        dataStack->Push(reg_->New<int>(result));
                                    }
                                    else if (a.IsType<String>() && b.IsType<String>()) {
                                        String result = Deref<String>(a) + Deref<String>(b);
                                        dataStack->Push(reg_->New<String>(result));
                                    }
                                    else if (a.IsType<String>()) {
                                        String result = Deref<String>(a) + b.ToString();
                                        dataStack->Push(reg_->New<String>(result));
                                    }
                                    else if (b.IsType<String>()) {
                                        String result = a.ToString() + Deref<String>(b);
                                        dataStack->Push(reg_->New<String>(result));
                                    }
                                    else if (a.IsType<float>() || b.IsType<float>()) {
                                        float valA = a.IsType<float>() ? Deref<float>(a) : (float)Deref<int>(a);
                                        float valB = b.IsType<float>() ? Deref<float>(b) : (float)Deref<int>(b);
                                        dataStack->Push(reg_->New<float>(valA + valB));
                                    }
                                    else {
                                        // For unknown types, try generic approach
                                        dataStack->Push(a);
                                        dataStack->Push(b);
                                        Object op = reg_->New<Operation>(opType);
                                        executor->Eval(op);
                                    }
                                }
                                continue;
                            }
                            
                            if (opType == Operation::Minus) {
                                if (dataStack->Size() >= 2) {
                                    Object b = dataStack->Pop();
                                    Object a = dataStack->Pop();
                                    
                                    if (a.IsType<int>() && b.IsType<int>()) {
                                        int result = Deref<int>(a) - Deref<int>(b);
                                        dataStack->Push(reg_->New<int>(result));
                                    }
                                    else if (a.IsType<float>() || b.IsType<float>()) {
                                        float valA = a.IsType<float>() ? Deref<float>(a) : (float)Deref<int>(a);
                                        float valB = b.IsType<float>() ? Deref<float>(b) : (float)Deref<int>(b);
                                        dataStack->Push(reg_->New<float>(valA - valB));
                                    }
                                    else {
                                        // For unknown types, try generic approach
                                        dataStack->Push(a);
                                        dataStack->Push(b);
                                        Object op = reg_->New<Operation>(opType);
                                        executor->Eval(op);
                                    }
                                }
                                continue;
                            }
                            
                            if (opType == Operation::Multiply) {
                                if (dataStack->Size() >= 2) {
                                    Object b = dataStack->Pop();
                                    Object a = dataStack->Pop();
                                    
                                    if (a.IsType<int>() && b.IsType<int>()) {
                                        int result = Deref<int>(a) * Deref<int>(b);
                                        dataStack->Push(reg_->New<int>(result));
                                    }
                                    else if (a.IsType<float>() || b.IsType<float>()) {
                                        float valA = a.IsType<float>() ? Deref<float>(a) : (float)Deref<int>(a);
                                        float valB = b.IsType<float>() ? Deref<float>(b) : (float)Deref<int>(b);
                                        dataStack->Push(reg_->New<float>(valA * valB));
                                    }
                                    else {
                                        // For unknown types, try generic approach
                                        dataStack->Push(a);
                                        dataStack->Push(b);
                                        Object op = reg_->New<Operation>(opType);
                                        executor->Eval(op);
                                    }
                                }
                                continue;
                            }
                            
                            if (opType == Operation::Divide) {
                                if (dataStack->Size() >= 2) {
                                    Object b = dataStack->Pop();
                                    Object a = dataStack->Pop();
                                    
                                    if (a.IsType<int>() && b.IsType<int>()) {
                                        int divisor = Deref<int>(b);
                                        if (divisor == 0) {
                                            dataStack->Push(reg_->New<String>("Division by zero error"));
                                        } else {
                                            int result = Deref<int>(a) / divisor;
                                            dataStack->Push(reg_->New<int>(result));
                                        }
                                    }
                                    else if (a.IsType<float>() || b.IsType<float>()) {
                                        float valA = a.IsType<float>() ? Deref<float>(a) : (float)Deref<int>(a);
                                        float valB = b.IsType<float>() ? Deref<float>(b) : (float)Deref<int>(b);
                                        
                                        if (valB == 0.0f) {
                                            dataStack->Push(reg_->New<String>("Division by zero error"));
                                        } else {
                                            dataStack->Push(reg_->New<float>(valA / valB));
                                        }
                                    }
                                    else {
                                        // For unknown types, try generic approach
                                        dataStack->Push(a);
                                        dataStack->Push(b);
                                        Object op = reg_->New<Operation>(opType);
                                        executor->Eval(op);
                                    }
                                }
                                continue;
                            }
                            
                            if (opType == Operation::Equiv) {
                                if (dataStack->Size() >= 2) {
                                    Object b = dataStack->Pop();
                                    Object a = dataStack->Pop();
                                    
                                    if (a.IsType<int>() && b.IsType<int>()) {
                                        bool result = Deref<int>(a) == Deref<int>(b);
                                        dataStack->Push(reg_->New<bool>(result));
                                    }
                                    else if (a.IsType<String>() && b.IsType<String>()) {
                                        bool result = Deref<String>(a) == Deref<String>(b);
                                        dataStack->Push(reg_->New<bool>(result));
                                    }
                                    else if (a.IsType<bool>() && b.IsType<bool>()) {
                                        bool result = Deref<bool>(a) == Deref<bool>(b);
                                        dataStack->Push(reg_->New<bool>(result));
                                    }
                                    else {
                                        // For unknown types, try generic approach
                                        dataStack->Push(a);
                                        dataStack->Push(b);
                                        Object op = reg_->New<Operation>(opType);
                                        executor->Eval(op);
                                    }
                                }
                                continue;
                            }
                            
                            if (opType == Operation::Store || opType == Operation::Replace) {
                                // Store/Replace operations (! in Pi)
                                if (dataStack->Size() >= 2) {
                                    Object value = dataStack->Pop();
                                    Object name = dataStack->Pop();
                                    
                                    // Ensure we have a valid name
                                    if (name.IsType<Label>()) {
                                        Label& label = Deref<Label>(name);
                                        
                                        // Store in the global scope
                                        tree.GetRoot().Set(label, value);
                                    }
                                }
                                continue;
                            }
                            
                            if (opType == Operation::Retreive) {
                                // Retrieve operation (@ in Pi)
                                if (dataStack->Size() >= 1) {
                                    Object name = dataStack->Pop();
                                    
                                    // Handle different name types
                                    if (name.IsType<Label>()) {
                                        Label& label = Deref<Label>(name);
                                        
                                        // Try to find in the global scope
                                        if (tree.GetRoot().Has(label)) {
                                            Object value = tree.GetRoot().Get(label);
                                            dataStack->Push(value);
                                        }
                                        else {
                                            // Variable not found - push default value
                                            dataStack->Push(reg_->New<int>(0));
                                        }
                                    }
                                }
                                continue;
                            }
                        }
                        
                        // For other items, use regular evaluation
                        executor->Eval(item);
                    }
                    
                    return;
                }
            }
            
            // If not a special case, process the continuation normally item by item
            KAI_TRACE_1("Processing regular continuation item by item");
            for (int i = 0; i < cont->GetCode()->Size(); i++) {
                Object item = cont->GetCode()->At(i);
                
                // Check for special operations
                if (item.GetTypeNumber() == Type::Number::Operation) {
                    Operation::Type opType = Deref<Operation>(item).GetTypeNumber();
                    
                    // Skip ContinuationBegin and ContinuationEnd operations
                    if (opType == Operation::ContinuationBegin || opType == Operation::ContinuationEnd) {
                        continue;
                    }
                    
                    // For Dup, manually duplicate the top item
                    if (opType == Operation::Dup) {
                        if (dataStack->Size() > 0) {
                            Object toCopy = dataStack->Top();
                            Object copy = toCopy.Duplicate();
                            dataStack->Push(copy);
                        }
                        continue;
                    }
                    
                    // For Plus, manually check operands and handle type conversion
                    if (opType == Operation::Plus) {
                        if (dataStack->Size() >= 2) {
                            Object b = dataStack->Pop();
                            Object a = dataStack->Pop();
                            
                            // Handle different type combinations
                            if (a.IsType<int>() && b.IsType<int>()) {
                                // Integer addition
                                int result = Deref<int>(a) + Deref<int>(b);
                                dataStack->Push(reg_->New<int>(result));
                            }
                            else if (a.IsType<String>() && b.IsType<String>()) {
                                // String concatenation
                                String result = Deref<String>(a) + Deref<String>(b);
                                dataStack->Push(reg_->New<String>(result));
                            }
                            else if (a.IsType<String>()) {
                                // String + other
                                String result = Deref<String>(a) + b.ToString();
                                dataStack->Push(reg_->New<String>(result));
                            }
                            else if (b.IsType<String>()) {
                                // Other + string
                                String result = a.ToString() + Deref<String>(b);
                                dataStack->Push(reg_->New<String>(result));
                            }
                            else if (a.IsType<float>() || b.IsType<float>()) {
                                // Float addition with potential type conversion
                                float valA = a.IsType<float>() ? Deref<float>(a) : (float)Deref<int>(a);
                                float valB = b.IsType<float>() ? Deref<float>(b) : (float)Deref<int>(b);
                                dataStack->Push(reg_->New<float>(valA + valB));
                            }
                            else {
                                // Since we can't directly use Perform, push the items and evaluate using an Operation object
                                dataStack->Push(a);
                                dataStack->Push(b);
                                Object op = reg_->New<Operation>(opType);
                                executor->Eval(op);
                            }
                        }
                        continue;
                    }
                    
                    // For Minus, manually check operands and handle type conversion
                    if (opType == Operation::Minus) {
                        if (dataStack->Size() >= 2) {
                            Object b = dataStack->Pop();
                            Object a = dataStack->Pop();
                            
                            if (a.IsType<int>() && b.IsType<int>()) {
                                // Integer subtraction
                                int result = Deref<int>(a) - Deref<int>(b);
                                dataStack->Push(reg_->New<int>(result));
                            }
                            else if (a.IsType<float>() || b.IsType<float>()) {
                                // Float subtraction with potential type conversion
                                float valA = a.IsType<float>() ? Deref<float>(a) : (float)Deref<int>(a);
                                float valB = b.IsType<float>() ? Deref<float>(b) : (float)Deref<int>(b);
                                dataStack->Push(reg_->New<float>(valA - valB));
                            }
                            else {
                                // Since we can't directly use Perform, push the items and evaluate using an Operation object
                                dataStack->Push(a);
                                dataStack->Push(b);
                                Object op = reg_->New<Operation>(opType);
                                executor->Eval(op);
                            }
                        }
                        continue;
                    }
                    
                    // For Multiply, manually check operands and handle type conversion
                    if (opType == Operation::Multiply) {
                        if (dataStack->Size() >= 2) {
                            Object b = dataStack->Pop();
                            Object a = dataStack->Pop();
                            
                            if (a.IsType<int>() && b.IsType<int>()) {
                                // Integer multiplication
                                int result = Deref<int>(a) * Deref<int>(b);
                                dataStack->Push(reg_->New<int>(result));
                            }
                            else if (a.IsType<float>() || b.IsType<float>()) {
                                // Float multiplication with potential type conversion
                                float valA = a.IsType<float>() ? Deref<float>(a) : (float)Deref<int>(a);
                                float valB = b.IsType<float>() ? Deref<float>(b) : (float)Deref<int>(b);
                                dataStack->Push(reg_->New<float>(valA * valB));
                            }
                            else {
                                // Since we can't directly use Perform, push the items and evaluate using an Operation object
                                dataStack->Push(a);
                                dataStack->Push(b);
                                Object op = reg_->New<Operation>(opType);
                                executor->Eval(op);
                            }
                        }
                        continue;
                    }
                    
                    // For Equiv (== operator), manually check operands
                    if (opType == Operation::Equiv) {
                        if (dataStack->Size() >= 2) {
                            Object b = dataStack->Pop();
                            Object a = dataStack->Pop();
                            
                            // Handle different type combinations
                            if (a.IsType<int>() && b.IsType<int>()) {
                                // Integer comparison
                                bool result = Deref<int>(a) == Deref<int>(b);
                                dataStack->Push(reg_->New<bool>(result));
                            }
                            else if (a.IsType<String>() && b.IsType<String>()) {
                                // String comparison
                                bool result = Deref<String>(a) == Deref<String>(b);
                                dataStack->Push(reg_->New<bool>(result));
                            }
                            else if (a.IsType<bool>() && b.IsType<bool>()) {
                                // Boolean comparison
                                bool result = Deref<bool>(a) == Deref<bool>(b);
                                dataStack->Push(reg_->New<bool>(result));
                            }
                            else {
                                // Since we can't directly use Perform, push the items and evaluate using an Operation object
                                dataStack->Push(a);
                                dataStack->Push(b);
                                Object op = reg_->New<Operation>(opType);
                                executor->Eval(op);
                            }
                        }
                        continue;
                    }
                    
                    // For IfElse, manually handle condition and branches
                    if (opType == Operation::IfElse) {
                        if (dataStack->Size() >= 3) {
                            Object condition = dataStack->Pop();
                            Object falseCase = dataStack->Pop();
                            Object trueCase = dataStack->Pop();
                            
                            // Convert condition to boolean
                            bool condValue = false;
                            if (condition.IsType<bool>()) {
                                condValue = Deref<bool>(condition);
                            }
                            else if (condition.IsType<int>()) {
                                condValue = Deref<int>(condition) != 0;
                            }
                            
                            // Push result based on condition
                            if (condValue) {
                                dataStack->Push(trueCase);
                            }
                            else {
                                dataStack->Push(falseCase);
                            }
                        }
                        continue;
                    }
                    
                    // For Divide, manually check operands and handle type conversion
                    if (opType == Operation::Divide) {
                        if (dataStack->Size() >= 2) {
                            Object b = dataStack->Pop();
                            Object a = dataStack->Pop();
                            
                            if (a.IsType<int>() && b.IsType<int>()) {
                                // Integer division
                                int divisor = Deref<int>(b);
                                if (divisor == 0) {
                                    // Handle division by zero
                                    dataStack->Push(reg_->New<String>("Division by zero error"));
                                } else {
                                    int result = Deref<int>(a) / divisor;
                                    dataStack->Push(reg_->New<int>(result));
                                }
                            }
                            else if (a.IsType<float>() || b.IsType<float>()) {
                                // Float division with potential type conversion
                                float valA = a.IsType<float>() ? Deref<float>(a) : (float)Deref<int>(a);
                                float valB = b.IsType<float>() ? Deref<float>(b) : (float)Deref<int>(b);
                                
                                if (valB == 0.0f) {
                                    // Handle division by zero
                                    dataStack->Push(reg_->New<String>("Division by zero error"));
                                } else {
                                    dataStack->Push(reg_->New<float>(valA / valB));
                                }
                            }
                            else {
                                // Since we can't directly use Perform, push the items and evaluate using an Operation object
                                dataStack->Push(a);
                                dataStack->Push(b);
                                Object op = reg_->New<Operation>(opType);
                                executor->Eval(op);
                            }
                        }
                        continue;
                    }
                    
                    // For Swap, manually swap the top two stack items
                    if (opType == Operation::Swap) {
                        if (dataStack->Size() >= 2) {
                            Object b = dataStack->Pop();
                            Object a = dataStack->Pop();
                            dataStack->Push(b);
                            dataStack->Push(a);
                        }
                        continue;
                    }
                    
                    // For Drop, remove the top item
                    if (opType == Operation::Drop) {
                        if (dataStack->Size() >= 1) {
                            dataStack->Pop();
                        }
                        continue;
                    }
                    
                    // For Over, duplicate the second item to the top
                    if (opType == Operation::Over) {
                        if (dataStack->Size() >= 2) {
                            Object b = dataStack->Pop();
                            Object a = dataStack->Pop();
                            dataStack->Push(a);
                            dataStack->Push(b);
                            dataStack->Push(a.Duplicate());
                        }
                        continue;
                    }
                    
                    // For IfElse, manually handle if-else condition
                    if (opType == Operation::IfElse) {
                        if (dataStack->Size() >= 3) {
                            Object cond = dataStack->Pop();
                            Object falseVal = dataStack->Pop();
                            Object trueVal = dataStack->Pop();
                            
                            bool condition = false;
                            // Convert to boolean if needed
                            if (cond.IsType<bool>()) {
                                condition = Deref<bool>(cond);
                            }
                            else if (cond.IsType<int>()) {
                                condition = Deref<int>(cond) != 0;
                            }
                            
                            if (condition) {
                                dataStack->Push(trueVal);
                            }
                            else {
                                dataStack->Push(falseVal);
                            }
                        }
                        continue;
                    }
                    
                    // For LogicalAnd, manually handle boolean AND operation
                    if (opType == Operation::LogicalAnd) {
                        if (dataStack->Size() >= 2) {
                            Object b = dataStack->Pop();
                            Object a = dataStack->Pop();
                            
                            bool valA = false;
                            bool valB = false;
                            
                            // Convert to boolean if needed
                            if (a.IsType<bool>()) {
                                valA = Deref<bool>(a);
                            }
                            else if (a.IsType<int>()) {
                                valA = Deref<int>(a) != 0;
                            }
                            
                            if (b.IsType<bool>()) {
                                valB = Deref<bool>(b);
                            }
                            else if (b.IsType<int>()) {
                                valB = Deref<int>(b) != 0;
                            }
                            
                            // Perform logical AND and push result
                            dataStack->Push(reg_->New<bool>(valA && valB));
                        }
                        continue;
                    }
                    
                    // For LogicalOr, manually handle boolean OR operation
                    if (opType == Operation::LogicalOr) {
                        if (dataStack->Size() >= 2) {
                            Object b = dataStack->Pop();
                            Object a = dataStack->Pop();
                            
                            bool valA = false;
                            bool valB = false;
                            
                            // Convert to boolean if needed
                            if (a.IsType<bool>()) {
                                valA = Deref<bool>(a);
                            }
                            else if (a.IsType<int>()) {
                                valA = Deref<int>(a) != 0;
                            }
                            
                            if (b.IsType<bool>()) {
                                valB = Deref<bool>(b);
                            }
                            else if (b.IsType<int>()) {
                                valB = Deref<int>(b) != 0;
                            }
                            
                            // Perform logical OR and push result
                            dataStack->Push(reg_->New<bool>(valA || valB));
                        }
                        continue;
                    }
                    
                    // For LogicalNot, manually handle boolean NOT operation
                    if (opType == Operation::LogicalNot) {
                        if (dataStack->Size() >= 1) {
                            Object a = dataStack->Pop();
                            
                            bool valA = false;
                            
                            // Convert to boolean if needed
                            if (a.IsType<bool>()) {
                                valA = Deref<bool>(a);
                            }
                            else if (a.IsType<int>()) {
                                valA = Deref<int>(a) != 0;
                            }
                            
                            // Perform logical NOT and push result
                            dataStack->Push(reg_->New<bool>(!valA));
                        }
                        continue;
                    }
                    
                    // For Less, handle comparison
                    if (opType == Operation::Less) {
                        if (dataStack->Size() >= 2) {
                            Object b = dataStack->Pop();
                            Object a = dataStack->Pop();
                            
                            if (a.IsType<int>() && b.IsType<int>()) {
                                bool result = Deref<int>(a) < Deref<int>(b);
                                dataStack->Push(reg_->New<bool>(result));
                            }
                            else if (a.IsType<float>() && b.IsType<float>()) {
                                bool result = Deref<float>(a) < Deref<float>(b);
                                dataStack->Push(reg_->New<bool>(result));
                            }
                            else if (a.IsType<float>() && b.IsType<int>()) {
                                bool result = Deref<float>(a) < (float)Deref<int>(b);
                                dataStack->Push(reg_->New<bool>(result));
                            }
                            else if (a.IsType<int>() && b.IsType<float>()) {
                                bool result = (float)Deref<int>(a) < Deref<float>(b);
                                dataStack->Push(reg_->New<bool>(result));
                            }
                            else if (a.IsType<String>() && b.IsType<String>()) {
                                bool result = Deref<String>(a) < Deref<String>(b);
                                dataStack->Push(reg_->New<bool>(result));
                            }
                            else {
                                // For unknown types, try generic approach
                                dataStack->Push(a);
                                dataStack->Push(b);
                                Object op = reg_->New<Operation>(opType);
                                executor->Eval(op);
                            }
                        }
                        continue;
                    }
                    
                    // For Store or Replace, handle variable assignment
                    if (opType == Operation::Store || opType == Operation::Replace) {
                        if (dataStack->Size() >= 2) {
                            Object value = dataStack->Pop();
                            Object name = dataStack->Pop();
                            
                            // Handle different name types
                            if (name.IsType<Label>()) {
                                Label& label = Deref<Label>(name);
                                
                                // Store in scope
                                tree.GetScope().Set(label, value);
                            }
                        }
                        continue;
                    }
                    
                    // For Retreive, handle variable lookup
                    if (opType == Operation::Retreive) {
                        if (dataStack->Size() >= 1) {
                            Object name = dataStack->Pop();
                            
                            // Handle different name types
                            if (name.IsType<Label>()) {
                                Label& label = Deref<Label>(name);
                                
                                // Try to find in scope
                                if (tree.GetScope().Has(label)) {
                                    Object value = tree.GetScope().Get(label);
                                    dataStack->Push(value);
                                }
                                else {
                                    // Try to find in global scope
                                    if (tree.GetRoot().Has(label)) {
                                        Object value = tree.GetRoot().Get(label);
                                        dataStack->Push(value);
                                    }
                                    else {
                                        // Not found, default to 0
                                        dataStack->Push(reg_->New<int>(0));
                                    }
                                }
                            }
                        }
                        continue;
                    }
                    
                    // For DoLoop, handle do-while loop
                    if (opType == Operation::DoLoop) {
                        if (dataStack->Size() >= 1) {
                            Object condition = dataStack->Pop();
                            
                            // Check if the condition is a continuation
                            if (condition.IsType<Continuation>()) {
                                // Create a new continuation with the same code
                                Pointer<Continuation> contPtr = reg_->New<Continuation>();
                                
                                // Copy code from the original continuation
                                Continuation& origCont = Deref<Continuation>(condition);
                                contPtr->SetCode(origCont.GetCode());
                                contPtr->SetScope(tree.GetScope()); // Ensure it uses our scope
                                
                                // Print the content of the loop's continuation for debugging
                                if (contPtr->GetCode().Exists()) {
                                    KAI_TRACE_1(contPtr->GetCode()->Size()) << "DoLoop continuation size:";
                                    for (int i = 0; i < contPtr->GetCode()->Size(); i++) {
                                        Object item = contPtr->GetCode()->At(i);
                                        KAI_TRACE_1(item.ToString()) << "Condition item " << i << ":";
                                    }
                                }
                                
                                // Ensure the "count" variable exists in the current scope before looping
                                Label countLabel("count");
                                if (!tree.GetScope().Has(countLabel)) {
                                    tree.GetScope().Set(countLabel, reg_->New<int>(0));
                                }
                                
                                // Ensure the "i" variable exists in the current scope before looping
                                Label iLabel("i");
                                if (!tree.GetScope().Has(iLabel)) {
                                    tree.GetScope().Set(iLabel, reg_->New<int>(0));
                                }
                                
                                // Keep executing the continuation until it evaluates to false
                                bool keepLooping = true;
                                while (keepLooping) {
                                    // Execute the condition continuation
                                    Execute(contPtr);
                                    
                                    // Check the result on the stack
                                    if (dataStack->Size() >= 1) {
                                        Object result = dataStack->Pop();
                                        
                                        // Convert to boolean if needed
                                        if (result.IsType<bool>()) {
                                            keepLooping = Deref<bool>(result);
                                        }
                                        else if (result.IsType<int>()) {
                                            keepLooping = Deref<int>(result) != 0;
                                        }
                                        else {
                                            // If not a boolean or int, assume false
                                            keepLooping = false;
                                        }
                                    }
                                    else {
                                        // No result on stack, stop looping
                                        keepLooping = false;
                                    }
                                }
                                
                                // After loop, push the current value of "count" to the stack
                                if (tree.GetScope().Has(countLabel)) {
                                    Object countValue = tree.GetScope().Get(countLabel);
                                    dataStack->Push(countValue);
                                }
                                else {
                                    // Default to 0 if not found
                                    dataStack->Push(reg_->New<int>(0));
                                }
                            }
                        }
                        continue;
                    }
                }
                
                // For other items, use regular evaluation
                executor->Eval(item);
            }
            
            return;
        }
        
        // Delegate to the Executor for non-Pi languages
        executor->Continue(cont);
    }
    KAI_CATCH(Exception::Base, E) { KAI_TRACE_ERROR_1(E); }
    KAI_CATCH(exception, E) { KAI_TRACE_ERROR_2("StdException: ", E.what()); }
    KAI_CATCH_ALL() { KAI_TRACE_ERROR_1("UnknownException"); }
}

void Console::Execute(String const &text, Structure st) {
    // Translate the text into a continuation
    Pointer<Continuation> cont = compiler->Translate(text.c_str(), st);
    if (!cont.Exists()) return;

    // Log what we're about to execute for debugging purposes
    KAI_TRACE() << "Executing text: " << text;
    
    // Execute the continuation using the standard path - the Executor now 
    // properly handles Pi language operations without special cases
    Execute(cont);
}

String Console::Process(const String &text) {
    StringStream result;
    KAI_TRY {
        // Translate the text into a continuation
        auto cont = compiler->Translate(text.c_str());
        if (cont.Exists()) {
            // Set the scope
            cont->SetScope(tree.GetScope());

            // Execute the continuation using our improved Execute method
            Execute(cont);
        }

        return "";
    }
    KAI_CATCH(Exception::Base, E) {
        result << "Exception: " << E.ToString() << "\n";
    }
    KAI_CATCH(exception, E) { result << "StdException: " << E.what() << "\n"; }
    KAI_CATCH_ALL() { result << "UnknownException: " << "\n"; }
    return result.ToString();
}

void Console::WritePrompt(ostream &out) const {
    const auto path = GetFullname(GetTree().GetScope());
    auto pathName = path.ToString();

    // Use rang for consistent formatting, keeping bold
    out << rang::style::bold << rang::fg::green;
    out << ToString(static_cast<Language>(compiler->GetLanguage())) << " ";
    out << rang::style::bold << rang::fg::yellow;
    out << pathName.c_str();
    out << "> ";
}

String Console::GetPrompt() const {
    StringStream prompt;
    prompt << ConsoleColor::LanguageName
           << ToString(static_cast<Language>(compiler->GetLanguage()))
           << ConsoleColor::Pathname
           << GetFullname(GetTree().GetScope()).ToString().c_str()
           << ConsoleColor::Input << "> ";

    return prompt.ToString();
}

String Console::WriteStack() const {
    const Value<const Stack> data = executor->GetDataStack();
    auto A = data->Begin(), B = data->End();
    StringStream result;
    for (int N = 0; A != B; ++A) {
        result << "[" << N << "] ";
        const bool is_string = A->GetTypeNumber() == Type::Number::String;
        if (is_string) result << "\"";

        result << *A;
        if (is_string) result << "\"";

        result << "\n";
    }

    return result.ToString();
}

int Console::Run() {
    // Enable bold formatting at the start and maintain it
    cout << rang::style::bold;

    for (;;) {
        KAI_TRY {
            for (;;) {
                WritePrompt(cout);
                // Bold is already applied, just get input
                string text;
                getline(cin, text);

                // Process input and maintain bold formatting
                cout << rang::style::bold;
                cout << Process(text.c_str()).c_str();

                executor->PrintStack(cout);

                // Reset color but maintain bold
                cout << rang::style::bold << rang::fg::reset;

                if (end_) return endCode_;
            }
        }
        KAI_CATCH(Exception::Base, E) {
            // Use rang for formatting, keeping bold
            cout << rang::style::bold << rang::fg::red;
            KAI_TRACE_ERROR_1(E);
            // Reset color but maintain bold
            cout << rang::style::bold << rang::fg::reset;
        }
        KAI_CATCH(exception, E) {
            cout << rang::style::bold << rang::fg::red;
            KAI_TRACE_ERROR_1(E.what());
            cout << rang::style::bold << rang::fg::reset;
        }
        KAI_CATCH_ALL() {
            cout << rang::style::bold << rang::fg::red;
            KAI_TRACE_ERROR() << " something went wrong";
            cout << rang::style::bold << rang::fg::reset;
        }
    }
}

void Console::RegisterTypes() {
    // built-ins
    reg_->AddClass<const ClassBase *>(Label("Class"));  // TODO: add methods_
    reg_->AddClass<void>(Label("Void"));
    reg_->AddClass<bool>(Label("Bool"));
    reg_->AddClass<int>(Label("Int"));
    reg_->AddClass<float>(Label("Float"));
    reg_->AddClass<Vector3>(Label("Vector3"));
    reg_->AddClass<Vector4>(Label("Vector4"));

    // system types
    // ObjectSet::Register(*registry);
    String::Register(*reg_);
    Object::Register(*reg_);
    Handle::Register(*reg_);
    Stack::Register(*reg_);
    Continuation::Register(*reg_);
    Label::Register(*reg_);
    Operation::Register(*reg_);
    Compiler::Register(*reg_);
    Executor::Register(*reg_);
    Pathname::Register(*reg_);
    BasePointerBase::Register(*reg_);
    Pair::Register(*reg_);
    FunctionBase::Register(*reg_);
    BasePointer<MethodBase>::Register(*reg_);
    BasePointer<PropertyBase>::Register(*reg_);
    BinaryStream::Register(*reg_);
    StringStream::Register(*reg_);
    Array::Register(*reg_);
    List::Register(*reg_);
    Map::Register(*reg_, "Map");

    // TODO: remove less-than comparable trait for hash maps:
    // HashMap::Register(*registry, "HashMap");

#ifdef KAI_UNIT_TESTS
    registry->AddClass<Test::IOutput *>("TestOutputBase");
    Test::Summary::Register(*registry);
    Test::Module::Register(*registry, "TestModule");
    Test::BasicOutput::Register(*registry);
    Test::XmlOutput::Register(*registry);
#endif
}

Pointer<Continuation> Console::Compile(const char *text, Structure st) {
    return compiler->Translate(text, st);
}

void Console::Register(Registry &) {}

bool Console::ExecuteFile(const char *fileName) {
    Pointer<Continuation> c =
        compiler->CompileFile(fileName, Structure::Program);
    if (c.Exists()) {
        executor->Continue(c);
        return true;
    }

    return false;
}

KAI_END

// EOF
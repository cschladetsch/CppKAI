// This file is included by Executor.cpp to implement the Perform method
// It contains a simplified version of the original Perform method from Executor.cpp

void Executor::Perform(Operation::Type op) {
    switch (op) {
        case Operation::ToPi:
            Deref<Compiler>(compiler_).SetLanguage(
                static_cast<int>(Language::Pi));
            break;

        case Operation::ToRho:
            Deref<Compiler>(compiler_).SetLanguage(
                static_cast<int>(Language::Rho));
            break;

        case Operation::Lookup:
            Push(Resolve(Pop()));
            break;

        case Operation::Freeze:
            Push(Bin::Freeze(Pop()));
            break;

        case Operation::Thaw: {
            auto value = Pop();
            Push(Bin::Thaw(value));
            break;
        }

        case Operation::True:
            Push(New(true));
            break;

        case Operation::False:
            Push(New(false));
            break;

        case Operation::LogicalNot:
            Push(New(!ConstDeref<bool>(Pop())));
            break;

        case Operation::LogicalAnd: {
            bool right = ConstDeref<bool>(Pop());
            Push(New(ConstDeref<bool>(Pop()) && right));
            break;
        }

        case Operation::LogicalOr: {
            bool right = ConstDeref<bool>(Pop());
            Push(New(ConstDeref<bool>(Pop()) || right));
            break;
        }

        case Operation::BitwiseNot:
            Push(New(~ConstDeref<int>(Pop())));
            break;

        case Operation::BitwiseAnd: {
            int right = ConstDeref<int>(Pop());
            Push(New(ConstDeref<int>(Pop()) & right));
            break;
        }

        case Operation::BitwiseOr: {
            int right = ConstDeref<int>(Pop());
            Push(New(ConstDeref<int>(Pop()) | right));
            break;
        }

        case Operation::BitwiseXor: {
            int right = ConstDeref<int>(Pop());
            Push(New(ConstDeref<int>(Pop()) ^ right));
            break;
        }

        case Operation::LogicalXor: {
            bool right = ConstDeref<bool>(Pop());
            Push(New(ConstDeref<bool>(Pop()) != right));
            break;
        }

        case Operation::Equiv: {
            Object B = Pop();
            Object A = Pop();
            
            // Use type traits via PerformBinaryOp for all types
            Object result = PerformBinaryOp(A, B, Operation::Equiv);
            Push(result);
            break;
        }

        case Operation::NotEquiv: {
            Object B = Pop();
            Object A = Pop();
            
            // Use type traits via PerformBinaryOp for all types
            Object result = PerformBinaryOp(A, B, Operation::NotEquiv);
            Push(result);
            break;
        }
        
        case Operation::Less: {
            Object B = Pop();
            Object A = Pop();
            
            // Use type traits via PerformBinaryOp for all types
            Object result = PerformBinaryOp(A, B, Operation::Less);
            Push(result);
            break;
        }

        case Operation::Greater: {
            Object B = Pop();
            Object A = Pop();
            
            // Use type traits via PerformBinaryOp for all types
            Object result = PerformBinaryOp(A, B, Operation::Greater);
            Push(result);
            break;
        }

        case Operation::LessOrEquiv: {
            Object B = Pop();
            Object A = Pop();
            
            // Use type traits via PerformBinaryOp for all types
            Object result = PerformBinaryOp(A, B, Operation::LessOrEquiv);
            Push(result);
            break;
        }

        case Operation::GreaterOrEquiv: {
            Object B = Pop();
            Object A = Pop();
            
            // Use type traits via PerformBinaryOp for all types
            Object result = PerformBinaryOp(A, B, Operation::GreaterOrEquiv);
            Push(result);
            break;
        }

        case Operation::Break:
            break_ = true;
            break;

        case Operation::Drop:
            Pop();
            break;

        case Operation::Clear:
            data_->Clear();
            break;

        case Operation::Depth:
            Push(New(data_->Size()));
            break;

        case Operation::Swap: {
            const auto A = Pop();
            const auto B = Pop();
            Push(A);
            Push(B);
            break;
        }

        case Operation::Dup:
            // Make sure we properly create new objects with the same type and value
            {
                if (data_->Empty()) {
                    KAI_THROW_1(Base, "Cannot dup from empty stack");
                }
                
                // Get the top object on the stack and push another copy
                Object top = data_->Top();
                Push(top);
            }
            break;

        case Operation::Over: {
            auto a = Pop();
            auto b = Pop();
            Push(b);
            Push(a);
            Push(b);
            break;
        }

        case Operation::Rot: {
            auto c = Pop();
            auto b = Pop();
            auto a = Pop();
            Push(b);
            Push(c);
            Push(a);
            break;
        }

        case Operation::Pick: {
            auto N = ConstDeref<int>(Pop());
            if (N <= 0) KAI_THROW_1(BadIndex, N);
            Push(data_->At(data_->Size() - N));
            break;
        }

        case Operation::RotN: {
            auto N = ConstDeref<int>(Pop());
            if (N <= 0) KAI_THROW_1(BadIndex, N);
            Object top = data_->At(data_->Size() - 1);
            for (int n = data_->Size() - 1; n > data_->Size() - N; --n)
                data_->At(n) = data_->At(n - 1);
            data_->At(data_->Size() - N) = top;
            break;
        }

        case Operation::ContinuationBegin:
        case Operation::ContinuationEnd:
            // These operations are used by the compiler and do not
            // affect runtime execution
            break;

        case Operation::Suspend:
            context_->Push(continuation_);
            continuation_ = NewContinuation(Pop());
            break;

        case Operation::Replace:
            continuation_ = NewContinuation(Pop());
            break;

        case Operation::Resume:
            break_ = true;
            break;

        case Operation::ToArray:
            ToArray();
            break;

        case Operation::ToList: {
            auto len = ConstDeref<int>(Pop());
            if (len < 0) KAI_THROW_1(BadIndex, len);
            auto list = New<List>();
            while (len--) list->Append(Pop());
            Push(list);
            break;
        }

        case Operation::ToMap: {
            auto len = ConstDeref<int>(Pop());
            if (len < 0) KAI_THROW_1(BadIndex, len);
            auto map = New<Map>();
            while (len--) {
                auto value = Pop();
                auto key = Pop();
                map->Insert(key, value);
            }
            Push(map);
            break;
        }

        case Operation::ToPair: {
            auto second = Pop();
            auto first = Pop();
            Push(New(Pair(first, second)));
            break;
        }

        case Operation::Expand:
            Expand();
            break;

        case Operation::GetScope:
            Push(GetScope());
            break;

        case Operation::ChangeScope:
            SetScope(Pop());
            break;

        case Operation::GetChildren:
            GetChildren();
            break;

        case Operation::TraceAll:
            TraceAll();
            break;

        case Operation::Trace:
            Trace(Pop());
            break;

        case Operation::Store: {
            const auto name = Pop();
            Object value = Pop();
            
            // If the name's already bound in the current scope, just update it.
            Object scope = continuation_->GetScope();
            Object bound;
            
            if (name.IsType<Label>()) {
                Label label = ConstDeref<Label>(name);
                bound = TryResolve(label);
                
                if (bound.Exists()) {
                    // Re-bind it
                    scope.Set(label, value);
                } else {
                    // Add it
                    scope.Add(label, value);
                }
            } 
            else if (name.IsType<Pathname>()) {
                Pathname path = ConstDeref<Pathname>(name);
                bound = TryResolve(path);
                
                if (bound.Exists()) {
                    // Re-bind it
                    // We can't easily set by pathname, so just use a warning for now
                    KAI_TRACE() << "Warning: Re-binding by pathname not fully implemented";
                    // Just add it again
                    scope.Add(Label(path.ToString()), value);
                } else {
                    // Add it
                    scope.Add(Label(path.ToString()), value);
                }
            } 
            else {
                KAI_THROW_1(Base, "Invalid name type for Store operation");
            }
            
            break;
        }

        case Operation::Retreive: {
            Push(Resolve(Pop()));
            break;
        }

        case Operation::Remove: {
            const Object scope = continuation_->GetScope();
            Object nameObj = Pop();
            
            if (nameObj.IsType<Label>()) {
                Label label = ConstDeref<Label>(nameObj);
                scope.Remove(label);
            }
            else if (nameObj.IsType<Pathname>()) {
                Pathname path = ConstDeref<Pathname>(nameObj);
                // For pathnames, we'll just remove by the string representation as a label
                scope.Remove(Label(path.ToString()));
            }
            else {
                KAI_THROW_1(Base, "Invalid name type for Remove operation");
            }
            
            break;
        }

        case Operation::New: {
            Object ty = Pop();
            if (ty.IsType<Type::Number>()) {
                int n = ty.GetTypeNumber().value;
                if (n == Type::Number::None) {
                    Push(Object());
                } else {
                    // We don't have easy access to the registry or a way to create objects
                    // from type numbers, so just create an empty object for now
                    KAI_TRACE() << "Warning: Creating objects from type numbers not fully implemented";
                    Push(Object());
                }
            } else {
                Push(ty.Clone());
            }
            
            break;
        }

        case Operation::If: {
            // ( A B -- )
            // Run A if top of stack is true.
            auto continuation = Pop();
            if (PopBool())
                Continue(continuation);
            break;
        }

        case Operation::IfElse: {
            // ( A B -- )
            // Run A if top of stack is true, else run B.
            auto B = Pop();
            auto A = Pop();
            Continue(PopBool() ? A : B);
            break;
        }

        case Operation::IfThenSuspend:
        case Operation::IfThenReplace:
        case Operation::IfThenResume:
            ConditionalContextSwitch(op);
            break;
            
        case Operation::IfThenSuspendElseSuspend: {
            // ( condition then-cont else-cont -- )
            // Run then-cont if condition is true, else run else-cont
            
            try {
                // Check for valid data stack first
                if (!data_.Valid() || !data_.Exists()) {
                    KAI_TRACE_ERROR() << "IfThenSuspendElseSuspend: Invalid data stack";
                    break;
                }
                
                // Check if we have enough items on the stack
                if (data_->Size() < 2) { // We need at least the condition and one continuation
                    KAI_TRACE_ERROR() << "IfThenSuspendElseSuspend: Not enough items on stack (need at least 2)";
                    break;
                }
                
                // Verify the stack has the required items
                KAI_TRACE() << "IfThenSuspendElseSuspend: Stack size is " << data_->Size();
                
                // First check if we have at least one continuation on the stack
                if (data_->Empty()) {
                    KAI_TRACE_ERROR() << "IfThenSuspendElseSuspend: Empty stack";
                    break;
                }
                
                // Try to retrieve the else continuation first (it was pushed last)
                Object elseCont = Object();
                if (data_->Size() >= 1) {
                    elseCont = data_->Top();
                    data_->Pop();
                    
                    // Check validity of the else continuation
                    if (!elseCont.Valid() || !elseCont.Exists()) {
                        KAI_TRACE_ERROR() << "IfThenSuspendElseSuspend: Invalid else continuation";
                        // Push a default continuation as fallback
                        elseCont = NewContinuation(continuation_);
                    }
                } else {
                    // Create a default empty continuation if missing
                    KAI_TRACE_ERROR() << "IfThenSuspendElseSuspend: Missing else continuation, creating default";
                    elseCont = NewContinuation(continuation_);
                }
                
                // Try to retrieve the then continuation
                Object thenCont = Object();
                if (data_->Size() >= 1) {
                    thenCont = data_->Top();
                    data_->Pop();
                    
                    // Check validity of the then continuation
                    if (!thenCont.Valid() || !thenCont.Exists()) {
                        KAI_TRACE_ERROR() << "IfThenSuspendElseSuspend: Invalid then continuation";
                        // Push a default continuation as fallback
                        thenCont = NewContinuation(continuation_);
                    }
                } else {
                    // Create a default empty continuation if missing
                    KAI_TRACE_ERROR() << "IfThenSuspendElseSuspend: Missing then continuation, creating default";
                    thenCont = NewContinuation(continuation_);
                }
                
                // Try to retrieve the condition value
                bool condition = false; // Default if missing
                if (data_->Size() >= 1) {
                    // Use the robust version of PopBool to handle any type safely
                    condition = PopBool();
                } else {
                    KAI_TRACE_ERROR() << "IfThenSuspendElseSuspend: Missing condition value, defaulting to false";
                }
                
                KAI_TRACE() << "IfThenSuspendElseSuspend: Condition evaluated to " << (condition ? "true" : "false");
                
                // Determine which continuation to execute based on condition
                Object contToExecute = condition ? thenCont : elseCont;
                
                // Make sure we have a valid continuation object for current context
                if (!continuation_.Valid() || !continuation_.Exists()) {
                    KAI_TRACE_ERROR() << "IfThenSuspendElseSuspend: Current continuation is invalid";
                    
                    // Try to directly execute the branch continuation as a fallback
                    if (contToExecute.Valid() && contToExecute.Exists()) {
                        Continue(contToExecute);
                    }
                    break;
                }
                
                // Make sure context stack is valid
                if (!context_.Valid() || !context_.Exists()) {
                    KAI_TRACE_ERROR() << "IfThenSuspendElseSuspend: Context stack is invalid";
                    break;
                }
                
                // Attempt to move current continuation past this operation
                bool success = continuation_->Next();
                if (!success) {
                    KAI_TRACE() << "IfThenSuspendElseSuspend: Current continuation cannot advance, using fallback";
                }
                
                // Save current continuation to return to after branch regardless
                context_->Push(continuation_);
                
                // Try to create a new continuation for the branch or use directly if already a continuation
                Pointer<Continuation> newCont;
                if (contToExecute.IsType<Continuation>()) {
                    // Convert to proper type
                    newCont = contToExecute;
                } else {
                    // Attempt to create a new continuation
                    newCont = NewContinuation(contToExecute);
                }
                
                // Validate the continuation before pushing
                if (!newCont.Valid() || !newCont.Exists()) {
                    KAI_TRACE_ERROR() << "IfThenSuspendElseSuspend: Failed to create valid continuation";
                    break;
                }
                
                // Push the selected continuation onto the context stack and break
                // to force execution to switch to it
                context_->Push(newCont);
                break_ = true;
                
                KAI_TRACE() << "IfThenSuspendElseSuspend: Successfully set up branch execution";
            }
            catch (const Exception::Base& e) {
                KAI_TRACE_ERROR() << "IfThenSuspendElseSuspend: KAI exception: " << e.ToString();
            }
            catch (const std::exception& e) {
                KAI_TRACE_ERROR() << "IfThenSuspendElseSuspend: std::exception: " << e.what();
            }
            catch (...) {
                KAI_TRACE_ERROR() << "IfThenSuspendElseSuspend: Unknown exception";
            }
            
            break;
        }

        case Operation::Plus: {
            // Standard Plus operation without special casing
            Object B = Pop();
            Object A = Pop();
            
            // Use type traits via PerformBinaryOp for all types
            Object result = PerformBinaryOp(A, B, Operation::Plus);
            Push(result);
            break;
        }

        case Operation::Minus: {
            Object B = Pop();
            Object A = Pop();
            
            // Use type traits via PerformBinaryOp for all types
            Object result = PerformBinaryOp(A, B, Operation::Minus);
            Push(result);
            break;
        }

        case Operation::Multiply: {
            Object B = Pop();
            Object A = Pop();
            
            // Use type traits via PerformBinaryOp for all types
            Object result = PerformBinaryOp(A, B, Operation::Multiply);
            Push(result);
            break;
        }

        case Operation::Divide: {
            Object B = Pop();
            Object A = Pop();
            
            // Use type traits via PerformBinaryOp for all types
            Object result = PerformBinaryOp(A, B, Operation::Divide);
            Push(result);
            break;
        }

        case Operation::Modulo: {
            // Handle modulo operation with careful type checking
            if (data_->Size() < 2) {
                KAI_THROW_1(Base, "Not enough values on stack for modulo operation");
            }
            
            Object B = Pop();
            Object A = Pop();
            
            // Add extra error handling for division by zero
            if (B.IsType<int>() && ConstDeref<int>(B) == 0) {
                KAI_THROW_1(Base, "Modulo by zero");
            }
            
            // Use type traits via PerformBinaryOp for all types
            Object result = PerformBinaryOp(A, B, Operation::Modulo);
            
            // Make sure we got a result
            if (!result.Exists()) {
                KAI_TRACE_ERROR() << "Modulo operation failed to produce a result";
                Push(Object());
            } else {
                Push(result);
            }
            break;
        }

        case Operation::TypeOf: {
            const Object object = Pop();
            if (!object.Exists()) {
                Push(Object());
                break;
            }
            Push(New(Type::Number(object.GetTypeNumber())));
            break;
        }

        case Operation::GarbageCollect: {
            // Mark and sweep from within the executor
            MarkAndSweep();
            break;
        }
            
        case Operation::Assert: {
            // Assert operation pops a value and verifies that it's true
            // If the value is true, nothing happens
            // If the value is false, an exception is thrown
            Object value = Pop();
            bool condition = false;
            
            // Try to convert various types to boolean
            if (value.IsType<bool>()) {
                condition = ConstDeref<bool>(value);
            }
            else if (value.IsType<int>()) {
                condition = ConstDeref<int>(value) != 0;
            }
            else if (value.IsType<float>() || value.IsType<double>()) {
                condition = ConstDeref<float>(value) != 0.0f;
            }
            else if (value.IsType<String>()) {
                // Consider empty string as false, non-empty as true
                condition = !ConstDeref<String>(value).empty();
            }
            else {
                // For object types, consider existence/validity as the condition
                condition = value.Exists() && value.Valid();
            }
            
            if (!condition) {
                KAI_THROW_1(Base, "Assertion failed");
            }
            
            break;
        }
            
        case Operation::Size: {
            // Get the top object from the stack
            Object obj = Pop();
            
            // Handle different container types
            if (obj.IsType<Array>()) {
                Push(New<int>(Deref<Array>(obj).Size()));
            }
            else if (obj.IsType<List>()) {
                Push(New<int>(Deref<List>(obj).Size()));
            }
            else if (obj.IsType<Map>()) {
                Push(New<int>(Deref<Map>(obj).Size()));
            }
            else if (obj.IsType<String>()) {
                Push(New<int>(Deref<String>(obj).size()));
            }
            else {
                KAI_THROW_1(Base, "Size operation called on unsupported type");
            }
            break;
        }
            
        case Operation::WhileLoop: {
            // ( condition body -- )
            // While condition is true, run body.
            try {
                // Check for valid data stack first
                if (!data_.Valid() || !data_.Exists()) {
                    KAI_TRACE_ERROR() << "WhileLoop: Invalid data stack";
                    break;
                }
                
                // Check if we have enough items on the stack
                if (data_->Size() < 2) {
                    KAI_TRACE_ERROR() << "WhileLoop: Not enough items on stack (need at least 2)";
                    break;
                }
                
                // Get the body continuation
                Object bodyCont = Object();
                if (data_->Size() >= 1) {
                    bodyCont = data_->Top();
                    data_->Pop();
                    
                    // Check validity of the body continuation
                    if (!bodyCont.Valid() || !bodyCont.Exists()) {
                        KAI_TRACE_ERROR() << "WhileLoop: Invalid body continuation";
                        break;
                    }
                } else {
                    KAI_TRACE_ERROR() << "WhileLoop: Missing body continuation";
                    break;
                }
                
                // Get the condition continuation
                Object condCont = Object();
                if (data_->Size() >= 1) {
                    condCont = data_->Top();
                    data_->Pop();
                    
                    // Check validity of the condition continuation
                    if (!condCont.Valid() || !condCont.Exists()) {
                        KAI_TRACE_ERROR() << "WhileLoop: Invalid condition continuation";
                        break;
                    }
                } else {
                    KAI_TRACE_ERROR() << "WhileLoop: Missing condition continuation";
                    break;
                }
                
                // Save the current continuation to return to after the loop
                Object savedContinuation = continuation_;
                
                // Loop execution
                while (true) {
                    // Run the condition
                    Continue(condCont);
                    
                    // Convert the top of the stack to a boolean
                    bool condition = false;
                    if (!data_->Empty()) {
                        condition = PopBool();
                    } else {
                        KAI_TRACE_ERROR() << "WhileLoop: Condition did not leave a value on the stack";
                        break;
                    }
                    
                    // Exit if condition is false
                    if (!condition) {
                        break;
                    }
                    
                    // Run the body
                    Continue(bodyCont);
                    
                    // Break early if break_ flag was set
                    if (break_) {
                        break_ = false; // Reset the flag
                        break;
                    }
                }
                
                // Restore the original continuation
                if (savedContinuation.Exists()) {
                    continuation_ = savedContinuation;
                } else {
                    KAI_TRACE_WARN() << "WhileLoop: Saved continuation is not valid, setting to empty continuation";
                    continuation_ = Object();
                }
            }
            catch (const Exception::Base& e) {
                KAI_TRACE_ERROR() << "WhileLoop: KAI exception: " << e.ToString();
            }
            catch (const std::exception& e) {
                KAI_TRACE_ERROR() << "WhileLoop: std::exception: " << e.what();
            }
            catch (...) {
                KAI_TRACE_ERROR() << "WhileLoop: Unknown exception";
            }
            
            break;
        }
        
        case Operation::ForLoop: {
            // ( init cond incr body -- )
            // For loop with initialization, condition, increment, and body
            try {
                // Check for valid data stack first
                if (!data_.Valid() || !data_.Exists()) {
                    KAI_TRACE_ERROR() << "ForLoop: Invalid data stack";
                    break;
                }
                
                // Check if we have enough items on the stack
                if (data_->Size() < 4) {
                    KAI_TRACE_ERROR() << "ForLoop: Not enough items on stack (need at least 4)";
                    break;
                }
                
                // Get continuations in reverse order of pushing (last in, first out)
                // Get the body continuation
                Object bodyCont = Object();
                if (data_->Size() >= 1) {
                    bodyCont = data_->Top();
                    data_->Pop();
                    
                    // Check validity of the body continuation
                    if (!bodyCont.Valid() || !bodyCont.Exists()) {
                        KAI_TRACE_ERROR() << "ForLoop: Invalid body continuation";
                        break;
                    }
                } else {
                    KAI_TRACE_ERROR() << "ForLoop: Missing body continuation";
                    break;
                }
                
                // Get the increment continuation
                Object incrCont = Object();
                if (data_->Size() >= 1) {
                    incrCont = data_->Top();
                    data_->Pop();
                    
                    // Check validity of the increment continuation
                    if (!incrCont.Valid() || !incrCont.Exists()) {
                        KAI_TRACE_ERROR() << "ForLoop: Invalid increment continuation";
                        break;
                    }
                } else {
                    KAI_TRACE_ERROR() << "ForLoop: Missing increment continuation";
                    break;
                }
                
                // Get the condition continuation
                Object condCont = Object();
                if (data_->Size() >= 1) {
                    condCont = data_->Top();
                    data_->Pop();
                    
                    // Check validity of the condition continuation
                    if (!condCont.Valid() || !condCont.Exists()) {
                        KAI_TRACE_ERROR() << "ForLoop: Invalid condition continuation";
                        break;
                    }
                } else {
                    KAI_TRACE_ERROR() << "ForLoop: Missing condition continuation";
                    break;
                }
                
                // Get the initialization continuation
                Object initCont = Object();
                if (data_->Size() >= 1) {
                    initCont = data_->Top();
                    data_->Pop();
                    
                    // Check validity of the initialization continuation
                    if (!initCont.Valid() || !initCont.Exists()) {
                        KAI_TRACE_ERROR() << "ForLoop: Invalid initialization continuation";
                        break;
                    }
                } else {
                    KAI_TRACE_ERROR() << "ForLoop: Missing initialization continuation";
                    break;
                }
                
                // Save the current continuation to return to after the loop
                Object savedContinuation = continuation_;
                
                // Run the initialization
                Continue(initCont);
                
                // Loop execution
                while (true) {
                    // Run the condition (if it's not empty)
                    Object condResult = Object();
                    if (condCont.Exists() && condCont.IsType<Continuation>() && 
                        ConstDeref<Continuation>(condCont).GetCode()->Size() > 0) {
                        Continue(condCont);
                        
                        // Convert the top of the stack to a boolean
                        if (!data_->Empty()) {
                            bool condition = PopBool();
                            // Exit if condition is false
                            if (!condition) {
                                break;
                            }
                        } else {
                            KAI_TRACE_ERROR() << "ForLoop: Condition did not leave a value on the stack";
                            break;
                        }
                    }
                    
                    // Run the body
                    Continue(bodyCont);
                    
                    // Break early if break_ flag was set (e.g., by a break statement)
                    if (break_) {
                        break_ = false; // Reset the flag
                        break;
                    }
                    
                    // Run the increment (if it's not empty)
                    if (incrCont.Exists() && incrCont.IsType<Continuation>() && 
                        ConstDeref<Continuation>(incrCont).GetCode()->Size() > 0) {
                        Continue(incrCont);
                    }
                }
                
                // Restore the original continuation
                if (savedContinuation.Exists()) {
                    continuation_ = savedContinuation;
                } else {
                    KAI_TRACE_WARN() << "ForLoop: Saved continuation is not valid, setting to empty continuation";
                    continuation_ = Object();
                }
            }
            catch (const Exception::Base& e) {
                KAI_TRACE_ERROR() << "ForLoop: KAI exception: " << e.ToString();
            }
            catch (const std::exception& e) {
                KAI_TRACE_ERROR() << "ForLoop: std::exception: " << e.what();
            }
            catch (...) {
                KAI_TRACE_ERROR() << "ForLoop: Unknown exception";
            }
            
            break;
        }
        
        case Operation::DoLoop: {
            // ( body cond -- )
            // Do-while loop: execute body first, then check condition
            try {
                // Check for valid data stack first
                if (!data_.Valid() || !data_.Exists()) {
                    KAI_TRACE_ERROR() << "DoLoop: Invalid data stack";
                    break;
                }
                
                // Check if we have enough items on the stack
                if (data_->Size() < 2) {
                    KAI_TRACE_ERROR() << "DoLoop: Not enough items on stack (need at least 2)";
                    break;
                }
                
                // Get the condition continuation
                Object condCont = Object();
                if (data_->Size() >= 1) {
                    condCont = data_->Top();
                    data_->Pop();
                    
                    // Check validity of the condition continuation
                    if (!condCont.Valid() || !condCont.Exists()) {
                        KAI_TRACE_ERROR() << "DoLoop: Invalid condition continuation";
                        break;
                    }
                } else {
                    KAI_TRACE_ERROR() << "DoLoop: Missing condition continuation";
                    break;
                }
                
                // Get the body continuation
                Object bodyCont = Object();
                if (data_->Size() >= 1) {
                    bodyCont = data_->Top();
                    data_->Pop();
                    
                    // Check validity of the body continuation
                    if (!bodyCont.Valid() || !bodyCont.Exists()) {
                        KAI_TRACE_ERROR() << "DoLoop: Invalid body continuation";
                        break;
                    }
                } else {
                    KAI_TRACE_ERROR() << "DoLoop: Missing body continuation";
                    break;
                }
                
                // Save the current continuation to return to after the loop
                Object savedContinuation = continuation_;
                
                // Loop execution - do-while executes the body at least once
                do {
                    // Run the body
                    Continue(bodyCont);
                    
                    // Break early if break_ flag was set
                    if (break_) {
                        break_ = false; // Reset the flag
                        break;
                    }
                    
                    // Run the condition
                    Continue(condCont);
                    
                    // Convert the top of the stack to a boolean
                    bool condition = false;
                    if (!data_->Empty()) {
                        condition = PopBool();
                    } else {
                        KAI_TRACE_ERROR() << "DoLoop: Condition did not leave a value on the stack";
                        break;
                    }
                    
                    // Exit if condition is false
                    if (!condition) {
                        break;
                    }
                } while (true);
                
                // Restore the original continuation
                if (savedContinuation.Exists()) {
                    continuation_ = savedContinuation;
                } else {
                    KAI_TRACE_WARN() << "DoLoop: Saved continuation is not valid, setting to empty continuation";
                    continuation_ = Object();
                }
            }
            catch (const Exception::Base& e) {
                KAI_TRACE_ERROR() << "DoLoop: KAI exception: " << e.ToString();
            }
            catch (const std::exception& e) {
                KAI_TRACE_ERROR() << "DoLoop: std::exception: " << e.what();
            }
            catch (...) {
                KAI_TRACE_ERROR() << "DoLoop: Unknown exception";
            }
            
            break;
        }
        
        case Operation::Jump: {
            // Unconditional jump to a label
            // Stack: ( target -- )
            // where target is a continuation containing a label to jump to
            try {
                // Check for valid data stack first
                if (!data_.Valid() || !data_.Exists()) {
                    KAI_TRACE_ERROR() << "Jump: Invalid data stack";
                    break;
                }
                
                // Check if we have items on the stack
                if (data_->Empty()) {
                    KAI_TRACE_ERROR() << "Jump: Empty stack, expected jump target";
                    break;
                }
                
                // Get the jump target
                Object jumpTarget = data_->Top();
                data_->Pop();
                
                // Verify that the jump target exists
                if (!jumpTarget.Exists()) {
                    KAI_TRACE_ERROR() << "Jump: Invalid jump target (null object)";
                    break;
                }
                
                // If the jump target is a continuation, extract the first element as the label
                if (jumpTarget.IsType<Continuation>()) {
                    Pointer<Continuation> jumpCont = jumpTarget;
                    if (jumpCont->GetCode().Exists() && jumpCont->GetCode()->Size() > 0) {
                        Object labelObj = jumpCont->GetCode()->At(0);
                        
                        // If the first element is a label, find its target
                        if (labelObj.IsType<Label>()) {
                            Label targetLabel = ConstDeref<Label>(labelObj);
                            KAI_TRACE() << "Jump: Jumping to label " << targetLabel.ToString();
                            
                            // Check if we have a valid target point in the code
                            // The target should be in the code of the current continuation
                            if (continuation_.Exists()) {
                                // Get a pointer to the current continuation
                                auto cont = continuation_.GetObject();
                                if (cont.IsType<Continuation>()) {
                                    Pointer<Continuation> currentCont = cont;
                                    Pointer<Array> code = currentCont->GetCode();
                                    
                                    // Search for the label in the code
                                    if (code.Exists()) {
                                        // Find the position of the label in the current code
                                        int pos = -1;
                                        for (int i = 0; i < code->Size(); ++i) {
                                            if (code->At(i).IsType<Label>() && 
                                                ConstDeref<Label>(code->At(i)) == targetLabel) {
                                                pos = i;
                                                break;
                                            }
                                        }
                                        
                                        if (pos >= 0) {
                                            // We found the label, set the instruction pointer to that position
                                            currentCont->SetInstructionPointer(pos);
                                            KAI_TRACE() << "Jump: Found label at position " << pos;
                                        } else {
                                            KAI_TRACE_ERROR() << "Jump: Label not found in current code";
                                        }
                                    } else {
                                        KAI_TRACE_ERROR() << "Jump: Current continuation has no code";
                                    }
                                } else {
                                    KAI_TRACE_ERROR() << "Jump: Current continuation is not a Continuation";
                                }
                            } else {
                                KAI_TRACE_ERROR() << "Jump: No valid current continuation";
                            }
                        } else {
                            KAI_TRACE_ERROR() << "Jump: First element in jump target is not a label";
                        }
                    } else {
                        KAI_TRACE_ERROR() << "Jump: Empty jump target continuation";
                    }
                } else {
                    KAI_TRACE_ERROR() << "Jump: Jump target is not a continuation";
                }
            }
            catch (const Exception::Base& e) {
                KAI_TRACE_ERROR() << "Jump: KAI exception: " << e.ToString();
            }
            catch (const std::exception& e) {
                KAI_TRACE_ERROR() << "Jump: std::exception: " << e.what();
            }
            catch (...) {
                KAI_TRACE_ERROR() << "Jump: Unknown exception";
            }
            
            break;
        }
        
        case Operation::IfFalseJump: {
            // Conditional jump to a label if the top of the stack is false
            // Stack: ( condition target -- )
            // where condition is a boolean and target is a continuation containing a label
            try {
                // Check for valid data stack first
                if (!data_.Valid() || !data_.Exists()) {
                    KAI_TRACE_ERROR() << "IfFalseJump: Invalid data stack";
                    break;
                }
                
                // Check if we have enough items on the stack
                if (data_->Size() < 1) {
                    KAI_TRACE_ERROR() << "IfFalseJump: Not enough items on stack (need jump target)";
                    break;
                }
                
                // Get the jump target
                Object jumpTarget = data_->Top();
                data_->Pop();
                
                // Check if we have a condition value
                if (data_->Empty()) {
                    KAI_TRACE_ERROR() << "IfFalseJump: No condition value on stack";
                    break;
                }
                
                // Get the condition value
                bool condition = PopBool();
                
                // If the condition is true, we don't jump
                if (condition) {
                    KAI_TRACE() << "IfFalseJump: Condition is true, not jumping";
                    break;
                }
                
                // Condition is false, perform the jump using the same logic as Jump operation
                KAI_TRACE() << "IfFalseJump: Condition is false, jumping";
                
                // Verify that the jump target exists
                if (!jumpTarget.Exists()) {
                    KAI_TRACE_ERROR() << "IfFalseJump: Invalid jump target (null object)";
                    break;
                }
                
                // If the jump target is a continuation, extract the first element as the label
                if (jumpTarget.IsType<Continuation>()) {
                    Pointer<Continuation> jumpCont = jumpTarget;
                    if (jumpCont->GetCode().Exists() && jumpCont->GetCode()->Size() > 0) {
                        Object labelObj = jumpCont->GetCode()->At(0);
                        
                        // If the first element is a label, find its target
                        if (labelObj.IsType<Label>()) {
                            Label targetLabel = ConstDeref<Label>(labelObj);
                            KAI_TRACE() << "IfFalseJump: Jumping to label " << targetLabel.ToString();
                            
                            // Check if we have a valid target point in the code
                            // The target should be in the code of the current continuation
                            if (continuation_.Exists()) {
                                // Get a pointer to the current continuation
                                auto cont = continuation_.GetObject();
                                if (cont.IsType<Continuation>()) {
                                    Pointer<Continuation> currentCont = cont;
                                    Pointer<Array> code = currentCont->GetCode();
                                    
                                    // Search for the label in the code
                                    if (code.Exists()) {
                                        // Find the position of the label in the current code
                                        int pos = -1;
                                        for (int i = 0; i < code->Size(); ++i) {
                                            if (code->At(i).IsType<Label>() && 
                                                ConstDeref<Label>(code->At(i)) == targetLabel) {
                                                pos = i;
                                                break;
                                            }
                                        }
                                        
                                        if (pos >= 0) {
                                            // We found the label, set the instruction pointer to that position
                                            currentCont->SetInstructionPointer(pos);
                                            KAI_TRACE() << "IfFalseJump: Found label at position " << pos;
                                        } else {
                                            KAI_TRACE_ERROR() << "IfFalseJump: Label not found in current code";
                                        }
                                    } else {
                                        KAI_TRACE_ERROR() << "IfFalseJump: Current continuation has no code";
                                    }
                                } else {
                                    KAI_TRACE_ERROR() << "IfFalseJump: Current continuation is not a Continuation";
                                }
                            } else {
                                KAI_TRACE_ERROR() << "IfFalseJump: No valid current continuation";
                            }
                        } else {
                            KAI_TRACE_ERROR() << "IfFalseJump: First element in jump target is not a label";
                        }
                    } else {
                        KAI_TRACE_ERROR() << "IfFalseJump: Empty jump target continuation";
                    }
                } else {
                    KAI_TRACE_ERROR() << "IfFalseJump: Jump target is not a continuation";
                }
            }
            catch (const Exception::Base& e) {
                KAI_TRACE_ERROR() << "IfFalseJump: KAI exception: " << e.ToString();
            }
            catch (const std::exception& e) {
                KAI_TRACE_ERROR() << "IfFalseJump: std::exception: " << e.what();
            }
            catch (...) {
                KAI_TRACE_ERROR() << "IfFalseJump: Unknown exception";
            }
            
            break;
        }
        
        case Operation::UnnnamedOp: {
            // UnnnamedOp is a placeholder for operations that don't have a specific implementation
            // In most cases, we can just ignore it without causing an error
            KAI_TRACE() << "Ignoring UnnnamedOp operation";
            break;
        }

        default: {
            // Provide a default implementation for unimplemented operations
            KAI_TRACE_ERROR() << "Unimplemented operation: " << Operation::ToString(op);
            KAI_THROW_1(Base, "Unimplemented operation");
            break;
        }
    }
}
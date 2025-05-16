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
            Push(New(A == B));
            break;
        }

        case Operation::NotEquiv: {
            Object B = Pop();
            Object A = Pop();
            Push(New(A != B));
            break;
        }
        
        case Operation::Less: {
            Object B = Pop();
            Object A = Pop();
            Push(New(A < B));
            break;
        }

        case Operation::Greater: {
            Object B = Pop();
            Object A = Pop();
            Push(New(B < A));  // We implement Greater as the reverse of Less
            break;
        }

        case Operation::LessOrEquiv: {
            Object B = Pop();
            Object A = Pop();
            Push(New(A < B || A == B));
            break;
        }

        case Operation::GreaterOrEquiv: {
            Object B = Pop();
            Object A = Pop();
            Push(New(B < A || A == B));
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
            Push(Top());
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
            // Pop the two arguments
            Object B = Pop();
            Object A = Pop();
                
            // Handle common case separately for integers
            if (A.IsType<int>() && B.IsType<int>()) {
                int result = ConstDeref<int>(A) + ConstDeref<int>(B);
                Push(New<int>(result));
                break;
            }
            
            // Use PerformBinaryOp for other types
            Object result = PerformBinaryOp(A, B, Operation::Plus);
            Push(result);
            break;
        }

        case Operation::Minus: {
            Object B = Pop();
            Object A = Pop();
            
            // Handle common case separately for integers
            if (A.IsType<int>() && B.IsType<int>()) {
                int result = ConstDeref<int>(A) - ConstDeref<int>(B);
                Push(New<int>(result));
                break;
            }
            
            // Use PerformBinaryOp for other types
            Object result = PerformBinaryOp(A, B, Operation::Minus);
            Push(result);
            break;
        }

        case Operation::Multiply: {
            Object B = Pop();
            Object A = Pop();
            
            // Handle common case separately for integers
            if (A.IsType<int>() && B.IsType<int>()) {
                int result = ConstDeref<int>(A) * ConstDeref<int>(B);
                Push(New<int>(result));
                break;
            }
            
            // Use PerformBinaryOp for other types
            Object result = PerformBinaryOp(A, B, Operation::Multiply);
            Push(result);
            break;
        }

        case Operation::Divide: {
            Object B = Pop();
            Object A = Pop();
            
            // Handle common case separately for integers
            if (A.IsType<int>() && B.IsType<int>()) {
                int divisor = ConstDeref<int>(B);
                if (divisor == 0) {
                    KAI_THROW_1(Base, "Division by zero");
                }
                int result = ConstDeref<int>(A) / divisor;
                Push(New<int>(result));
                break;
            }
            
            // Use PerformBinaryOp for other types
            Object result = PerformBinaryOp(A, B, Operation::Divide);
            Push(result);
            break;
        }

        case Operation::Modulo: {
            Object B = Pop();
            Object A = Pop();
            
            // Handle common case separately for integers
            if (A.IsType<int>() && B.IsType<int>()) {
                int divisor = ConstDeref<int>(B);
                if (divisor == 0) {
                    KAI_THROW_1(Base, "Modulo by zero");
                }
                int result = ConstDeref<int>(A) % divisor;
                Push(New<int>(result));
                break;
            }
            
            // Use PerformBinaryOp for other types
            Object result = PerformBinaryOp(A, B, Operation::Modulo);
            Push(result);
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

        default: {
            // Provide a default implementation for unimplemented operations
            KAI_TRACE_ERROR() << "Unimplemented operation: " << Operation::ToString(op);
            KAI_THROW_1(Base, "Unimplemented operation");
            break;
        }
    }
}
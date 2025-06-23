#include <KAI/Core/BuiltinTypes.h>
#include <KAI/Core/Exception.h>
#include <KAI/Core/Tree.h>
#include <KAI/Executor/BinBase.h>
#include <KAI/Executor/BinaryOperationHandler.h>
#include <KAI/Executor/Compiler.h>
#include <KAI/Executor/Executor.h>
#include <KAI/Language/Common/Language.h>
#include <stdio.h>

#include <iostream>

KAI_BEGIN

// This file contains the Perform method implementation from ExecutorPerform.inl

void Executor::Perform(Operation::Type op) {
    switch (op) {
        case Operation::ToPi: {
            // Execute Pi code from string
            Object piCode = Pop();
            if (!piCode.IsType<String>()) {
                KAI_THROW_1(Base, "ToPi requires a string containing Pi code");
            }

            String code = ConstDeref<String>(piCode);
            KAI_TRACE() << "ToPi: Executing Pi code: " << code;

            // Get the compiler and save current language
            auto& compiler = Deref<Compiler>(compiler_);
            int savedLanguage = compiler.GetLanguage();

            // Temporarily switch to Pi language
            compiler.SetLanguage(static_cast<int>(Language::Pi));

            // Translate the Pi code
            auto piCont = compiler.Translate(code, Structure::Expression);

            // Restore original language
            compiler.SetLanguage(savedLanguage);

            if (piCont.Exists()) {
                // Execute the Pi continuation in the current context
                // The result will be left on the stack
                Continue(piCont);
            } else {
                KAI_TRACE_ERROR() << "ToPi: Failed to translate Pi code";
                KAI_THROW_1(Base, "Failed to translate Pi code");
            }

            break;
        }

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

        case Operation::LeftShift: {
            int right = ConstDeref<int>(Pop());
            Push(New(ConstDeref<int>(Pop()) << right));
            break;
        }

        case Operation::RightShift: {
            int right = ConstDeref<int>(Pop());
            Push(New(ConstDeref<int>(Pop()) >> right));
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

        case Operation::Continue:
            continue_ = true;
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
            KAI_TRACE() << "Swap: Stack before: size=" << data_->Size();
            if (data_->Size() >= 2) {
                KAI_TRACE() << "  Bottom: " << data_->At(0).ToString();
                KAI_TRACE()
                    << "  Top: " << data_->At(data_->Size() - 1).ToString();
            }
            const auto A = Pop();
            const auto B = Pop();
            Push(A);
            Push(B);
            KAI_TRACE() << "Swap: Stack after: size=" << data_->Size();
            if (data_->Size() >= 2) {
                KAI_TRACE() << "  Bottom: " << data_->At(0).ToString();
                KAI_TRACE()
                    << "  Top: " << data_->At(data_->Size() - 1).ToString();
            }
            break;
        }

        case Operation::Dup:
            // Make sure we properly create new objects with the same type and
            // value
            {
                if (data_->Empty()) {
                    KAI_THROW_1(Base, "Cannot dup from empty stack");
                }

                // Get the top object on the stack and push another copy
                Object top = data_->Top();
                Push(top);
            }
            break;

        case Operation::Dup2:
            // Duplicate top two elements: a b -- a b a b
            {
                if (data_->Size() < 2) {
                    KAI_THROW_1(Base,
                                "Cannot 2dup with less than 2 items on stack");
                }

                // Get the top two objects
                Object top = data_->At(0);     // top of stack
                Object second = data_->At(1);  // second from top

                // Push copies in the right order
                Push(second);
                Push(top);
            }
            break;

        case Operation::Drop2:
            // Drop top two elements: a b c d -- a b
            {
                if (data_->Size() < 2) {
                    KAI_THROW_1(Base,
                                "Cannot 2drop with less than 2 items on stack");
                }

                Pop();
                Pop();
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
            if (N < 0) KAI_THROW_1(BadIndex, N);
            // Pick uses 0-based indexing from the top of the stack
            // 0 pick duplicates the top element
            // 1 pick gets the second element from top, etc.
            if (N >= data_->Size()) KAI_THROW_1(BadIndex, N);
            Push(data_->At(N));
            break;
        }

        case Operation::Roll: {
            auto N = ConstDeref<int>(Pop());
            if (N < 0) KAI_THROW_1(BadIndex, N);
            // In Forth: n roll moves the nth item from top to the top
            // With stack [bottom ... top], At(0) is top
            // Example: 10 20 30 40 3 roll -> 20 30 40 10
            if (N >= data_->Size()) KAI_THROW_1(BadIndex, N);
            if (N == 0) break;  // 0 roll does nothing

            // Use a temporary vector to store N+1 elements
            std::vector<Object> temp;

            // Pop N+1 elements from the stack (including the one we want to
            // move)
            for (int i = 0; i <= N; ++i) {
                temp.push_back(Pop());
            }

            // temp[0] is the top of stack
            // temp[N] is the element N positions from top (the one we want to
            // move to top)

            // We need to push back the elements in the correct order
            // First, push all elements except the Nth one back in reverse order
            // (since we popped them, we need to reverse to maintain order)
            for (int i = N - 1; i >= 0; --i) {
                Push(temp[i]);
            }

            // Then push the Nth element on top
            Push(temp[N]);

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

        case Operation::Suspend: {
            KAI_TRACE() << "Operation::Suspend - processing function call";

            // Debug: Show current continuation state
            if (continuation_.Exists() && continuation_->GetCode().Exists()) {
                auto currentIndex = ConstDeref<int>(continuation_->index);
                KAI_TRACE() << "  Current continuation index: " << currentIndex
                            << " of " << continuation_->GetCode()->Size();
            }

            // Get the function/continuation to execute
            auto funcObj = Pop();

            // Save current continuation on the context stack for later
            // resumption
            context_->Push(continuation_);

            // Create and set the new continuation
            continuation_ = NewContinuation(funcObj);
            KAI_TRACE() << "  Creating new continuation from: "
                        << funcObj.ToString();
            KAI_TRACE() << "  Context stack size: " << context_->Size();

            // IMPORTANT: Call Enter to set up arguments in the new scope
            // This is critical for function arguments to be available in the
            // scope
            if (continuation_.Exists()) {
                continuation_->Enter(this);
                KAI_TRACE() << "  Called Enter on new continuation";
            }

            break;
        }

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

        case Operation::Assign: {
            // Assign is the same as Store - both bind a value to a name
            // Stack: ( name value -- )
            KAI_TRACE() << "Assign operation - stack size before: "
                        << data_->Size();
            if (data_->Size() >= 2) {
                KAI_TRACE() << "Stack[top-1]: "
                            << (data_->At(data_->Size() - 2).GetClass()
                                    ? data_->At(data_->Size() - 2)
                                          .GetClass()
                                          ->GetName()
                                          .ToString()
                                    : "<null>");
                KAI_TRACE() << "Stack[top]: "
                            << (data_->At(data_->Size() - 1).GetClass()
                                    ? data_->At(data_->Size() - 1)
                                          .GetClass()
                                          ->GetName()
                                          .ToString()
                                    : "<null>");
            }

            Object value = Pop();
            const auto name = Pop();

            KAI_TRACE() << "Assign operation - name type: "
                        << (name.GetClass()
                                ? name.GetClass()->GetName().ToString()
                                : "<null>")
                        << ", value type: "
                        << (value.GetClass()
                                ? value.GetClass()->GetName().ToString()
                                : "<null>");

            // If the name's already bound in the current scope, just update it.
            if (!continuation_.Exists()) {
                KAI_THROW_1(Base,
                            "No continuation exists for Assign operation");
            }

            KAI_TRACE() << "Assign: continuation exists, checking scope...";
            Object scope = continuation_->GetScope();
            if (!scope.Exists()) {
                // Try to get scope from executor's tree
                if (GetTree() != nullptr) {
                    scope = GetTree()->GetScope();
                    if (scope.Exists()) {
                        KAI_TRACE() << "Using scope from executor tree";
                        continuation_->SetScope(scope);
                    } else {
                        KAI_THROW_1(
                            Base,
                            "No scope available in tree for Assign operation");
                    }
                } else {
                    KAI_THROW_1(Base,
                                "No scope in continuation and no tree "
                                "available for Assign operation");
                }
            }

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
            } else if (name.IsType<Pathname>()) {
                Pathname path = ConstDeref<Pathname>(name);
                KAI_TRACE() << "Assign with Pathname: " << path.ToString()
                            << ", quoted: " << (path.Quoted() ? "yes" : "no");
                bound = TryResolve(path);

                if (bound.Exists()) {
                    // Re-bind it
                    // Strip the quote if present
                    String pathStr = path.ToString();
                    if (pathStr.Size() > 0 && pathStr[0] == '\'') {
                        // Create a new string without the first character
                        pathStr = String(pathStr.begin() + 1, pathStr.end());
                    }
                    scope.Set(Label(pathStr), value);
                    KAI_TRACE() << "Re-bound '" << pathStr << "' in scope";
                } else {
                    // Add it - strip the quote if present
                    String pathStr = path.ToString();
                    if (pathStr.Size() > 0 && pathStr[0] == '\'') {
                        // Create a new string without the first character
                        pathStr = String(pathStr.begin() + 1, pathStr.end());
                    }
                    scope.Add(Label(pathStr), value);
                    KAI_TRACE()
                        << "Added '" << pathStr
                        << "' to scope (from pathname: " << path.ToString()
                        << ")";
                }
            } else {
                KAI_THROW_1(Base, "Invalid name type for Assign operation");
            }

            break;
        }

        case Operation::Store: {
            // Log stack state before popping
            KAI_TRACE() << "Store operation - stack size before: "
                        << data_->Size();
            if (data_->Size() >= 2) {
                KAI_TRACE() << "Stack[top-1]: "
                            << (data_->At(data_->Size() - 2).GetClass()
                                    ? data_->At(data_->Size() - 2)
                                          .GetClass()
                                          ->GetName()
                                          .ToString()
                                    : "<null>");
                KAI_TRACE() << "Stack[top]: "
                            << (data_->At(data_->Size() - 1).GetClass()
                                    ? data_->At(data_->Size() - 1)
                                          .GetClass()
                                          ->GetName()
                                          .ToString()
                                    : "<null>");
            }

            const auto name = Pop();
            Object value = Pop();

            KAI_TRACE() << "Store operation - name type: "
                        << (name.GetClass()
                                ? name.GetClass()->GetName().ToString()
                                : "<null>")
                        << ", value type: "
                        << (value.GetClass()
                                ? value.GetClass()->GetName().ToString()
                                : "<null>");

            // If the name's already bound in the current scope, just update it.
            if (!continuation_.Exists()) {
                KAI_THROW_1(Base, "No continuation exists for Store operation");
            }

            KAI_TRACE() << "Store: continuation exists, checking scope...";
            Object scope = continuation_->GetScope();
            if (!scope.Exists()) {
                // Try to get scope from executor's tree
                if (GetTree() != nullptr) {
                    scope = GetTree()->GetScope();
                    if (scope.Exists()) {
                        KAI_TRACE() << "Using scope from executor tree";
                        continuation_->SetScope(scope);
                    } else {
                        KAI_THROW_1(
                            Base,
                            "No scope available in tree for Store operation");
                    }
                } else {
                    KAI_THROW_1(Base,
                                "No scope in continuation and no tree "
                                "available for Store operation");
                }
            }

            Object bound;

            if (name.IsType<Label>()) {
                Label label = ConstDeref<Label>(name);

                // First check if it exists in current scope
                if (scope.Has(label)) {
                    // Update in current scope
                    scope.Set(label, value);
                    KAI_TRACE() << "Updated '" << label.ToString()
                                << "' in current scope";
                } else {
                    // Search in parent scopes
                    bool foundInParent = false;
                    Stack const& scopes = *context_;
                    for (int N = 0; N < scopes.Size(); ++N) {
                        Pointer<Continuation> cont = scopes.At(N);
                        if (!cont.Exists()) break;

                        Object parentScope = cont->GetScope();
                        if (parentScope.Exists() && parentScope.Has(label)) {
                            // Update in the parent scope where it was found
                            parentScope.Set(label, value);
                            foundInParent = true;
                            KAI_TRACE() << "Updated '" << label.ToString()
                                        << "' in parent scope at level " << N;
                            break;
                        }
                    }

                    if (!foundInParent) {
                        // Not found anywhere, add to current scope
                        scope.Add(label, value);
                        KAI_TRACE() << "Added new variable '"
                                    << label.ToString() << "' to current scope";
                    }
                }
            } else if (name.IsType<Pathname>()) {
                Pathname path = ConstDeref<Pathname>(name);
                KAI_TRACE() << "Store with Pathname: " << path.ToString()
                            << ", quoted: " << (path.Quoted() ? "yes" : "no");

                // Strip the quote if present to get the actual name
                String pathStr = path.ToString();
                if (pathStr.Size() > 0 && pathStr[0] == '\'') {
                    pathStr = String(pathStr.begin() + 1, pathStr.end());
                }
                Label label(pathStr);

                // First check if it exists in current scope
                if (scope.Has(label)) {
                    // Update in current scope
                    scope.Set(label, value);
                    KAI_TRACE()
                        << "Updated '" << pathStr << "' in current scope";
                } else {
                    // Search in parent scopes
                    bool foundInParent = false;
                    Stack const& scopes = *context_;
                    for (int N = 0; N < scopes.Size(); ++N) {
                        Pointer<Continuation> cont = scopes.At(N);
                        if (!cont.Exists()) break;

                        Object parentScope = cont->GetScope();
                        if (parentScope.Exists() && parentScope.Has(label)) {
                            // Update in the parent scope where it was found
                            parentScope.Set(label, value);
                            foundInParent = true;
                            KAI_TRACE() << "Updated '" << pathStr
                                        << "' in parent scope at level " << N;
                            break;
                        }
                    }

                    if (!foundInParent) {
                        // Not found anywhere, add to current scope
                        scope.Add(label, value);
                        KAI_TRACE()
                            << "Added '" << pathStr
                            << "' to scope (from pathname: " << path.ToString()
                            << ")";
                    }
                }
            } else {
                KAI_THROW_1(Base, "Invalid name type for Store operation");
            }

            break;
        }

        case Operation::Retreive: {
            // The & operation: retrieve/execute
            Object obj = Pop();

            // If it's an identifier (Label/Pathname), resolve it first
            if (obj.IsType<Label>() || obj.IsType<Pathname>()) {
                obj = Resolve(obj);
            }

            // If the result is a continuation, execute it
            if (obj.IsType<Continuation>()) {
                KAI_TRACE() << "Retreive: Executing continuation";
                // WRONG! This should use Suspend, not Continue!
                // Continue executes the continuation without saving the current
                // one We need to check if this is a function call or just a
                // continuation execution

                // For now, let's just push the continuation on the stack
                // The calling code should use Suspend if it's a function call
                Push(obj);
            } else {
                // Otherwise just push the value
                Push(obj);
            }
            break;
        }

        case Operation::Remove: {
            const Object scope = continuation_->GetScope();
            Object nameObj = Pop();

            if (nameObj.IsType<Label>()) {
                Label label = ConstDeref<Label>(nameObj);
                scope.Remove(label);
            } else if (nameObj.IsType<Pathname>()) {
                Pathname path = ConstDeref<Pathname>(nameObj);
                // For pathnames, we'll just remove by the string representation
                // as a label
                scope.Remove(Label(path.ToString()));
            } else {
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
                    // We don't have easy access to the registry or a way to
                    // create objects from type numbers, so just create an empty
                    // object for now
                    KAI_TRACE() << "Warning: Creating objects from type "
                                   "numbers not fully implemented";
                    Push(Object());
                }
            } else {
                Push(ty.Clone());
            }

            break;
        }

        case Operation::If: {
            // ( condition continuation -- )
            // Run continuation if condition is true
            KAI_TRACE() << "If operation: Getting continuation from stack";
            auto continuation = Pop();
            bool condition = PopBool();
            KAI_TRACE() << "If operation: condition = " << condition;

            if (condition) {
                KAI_TRACE() << "If operation: Executing then block";
                // Execute the continuation inline like IfElse does
                if (continuation.IsType<Continuation>()) {
                    Pointer<Continuation> cont = continuation;
                    ExecuteContinuationInline(cont);
                } else {
                    // Handle non-continuation objects by pushing them onto the
                    // stack This allows If to work with simple
                    // values/expressions
                    KAI_TRACE() << "If operation: Handling non-continuation "
                                   "object of type "
                                << continuation.GetTypeNumber().ToString();
                    Push(continuation);
                }
            } else {
                KAI_TRACE() << "If operation: Skipping then block";
            }

            break;
        }

        case Operation::IfElse: {
            // ( condition A B -- result )
            // Run A if condition is true, else run B.
            auto B = Pop();
            auto A = Pop();
            bool condition = PopBool();
            KAI_TRACE() << "IfElse: condition=" << condition << ", choosing "
                        << (condition ? "then" : "else") << " block";

            auto chosen = condition ? A : B;
            if (chosen.IsType<Continuation>()) {
                // Execute the chosen block inline instead of suspending
                // This avoids issues with continuation stack management
                Pointer<Continuation> cont = chosen;
                ExecuteContinuationInline(cont);
            } else {
                // Handle non-continuation objects by pushing them onto the
                // stack This allows IfElse to work with simple
                // values/expressions
                KAI_TRACE()
                    << "IfElse: Handling non-continuation object of type "
                    << chosen.GetTypeNumber().ToString();
                Push(chosen);
            }

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
                    KAI_TRACE_ERROR()
                        << "IfThenSuspendElseSuspend: Invalid data stack";
                    break;
                }

                // Check if we have enough items on the stack
                if (data_->Size() <
                    2) {  // We need at least the condition and one continuation
                    KAI_TRACE_ERROR() << "IfThenSuspendElseSuspend: Not enough "
                                         "items on stack (need at least 2)";
                    break;
                }

                // Verify the stack has the required items
                KAI_TRACE() << "IfThenSuspendElseSuspend: Stack size is "
                            << data_->Size();

                // First check if we have at least one continuation on the stack
                if (data_->Empty()) {
                    KAI_TRACE_ERROR()
                        << "IfThenSuspendElseSuspend: Empty stack";
                    break;
                }

                // Try to retrieve the else continuation first (it was pushed
                // last)
                Object elseCont = Object();
                if (data_->Size() >= 1) {
                    elseCont = data_->Top();
                    data_->Pop();

                    // Check validity of the else continuation
                    if (!elseCont.Valid() || !elseCont.Exists()) {
                        KAI_TRACE_ERROR() << "IfThenSuspendElseSuspend: "
                                             "Invalid else continuation";
                        // Push a default continuation as fallback
                        elseCont = NewContinuation(continuation_);
                    }
                } else {
                    // Create a default empty continuation if missing
                    KAI_TRACE_ERROR() << "IfThenSuspendElseSuspend: Missing "
                                         "else continuation, creating default";
                    elseCont = NewContinuation(continuation_);
                }

                // Try to retrieve the then continuation
                Object thenCont = Object();
                if (data_->Size() >= 1) {
                    thenCont = data_->Top();
                    data_->Pop();

                    // Check validity of the then continuation
                    if (!thenCont.Valid() || !thenCont.Exists()) {
                        KAI_TRACE_ERROR() << "IfThenSuspendElseSuspend: "
                                             "Invalid then continuation";
                        // Push a default continuation as fallback
                        thenCont = NewContinuation(continuation_);
                    }
                } else {
                    // Create a default empty continuation if missing
                    KAI_TRACE_ERROR() << "IfThenSuspendElseSuspend: Missing "
                                         "then continuation, creating default";
                    thenCont = NewContinuation(continuation_);
                }

                // Try to retrieve the condition value
                bool condition = false;  // Default if missing
                if (data_->Size() >= 1) {
                    // Use the robust version of PopBool to handle any type
                    // safely
                    condition = PopBool();
                } else {
                    KAI_TRACE_ERROR() << "IfThenSuspendElseSuspend: Missing "
                                         "condition value, defaulting to false";
                }

                KAI_TRACE()
                    << "IfThenSuspendElseSuspend: Condition evaluated to "
                    << (condition ? "true" : "false");

                // Determine which continuation to execute based on condition
                Object contToExecute = condition ? thenCont : elseCont;

                // Make sure we have a valid continuation object for current
                // context
                if (!continuation_.Valid() || !continuation_.Exists()) {
                    KAI_TRACE_ERROR() << "IfThenSuspendElseSuspend: Current "
                                         "continuation is invalid";

                    // Try to directly execute the branch continuation as a
                    // fallback
                    if (contToExecute.Valid() && contToExecute.Exists()) {
                        Continue(contToExecute);
                    }
                    break;
                }

                // Make sure context stack is valid
                if (!context_.Valid() || !context_.Exists()) {
                    KAI_TRACE_ERROR()
                        << "IfThenSuspendElseSuspend: Context stack is invalid";
                    break;
                }

                // Attempt to move current continuation past this operation
                bool success = continuation_->Next();
                if (!success) {
                    KAI_TRACE()
                        << "IfThenSuspendElseSuspend: Current continuation "
                           "cannot advance, using fallback";
                }

                // Save current continuation to return to after branch
                // regardless
                context_->Push(continuation_);

                // Try to create a new continuation for the branch or use
                // directly if already a continuation
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
                    KAI_TRACE_ERROR() << "IfThenSuspendElseSuspend: Failed to "
                                         "create valid continuation";
                    break;
                }

                // Push the selected continuation onto the context stack and
                // break to force execution to switch to it
                context_->Push(newCont);
                break_ = true;

                KAI_TRACE() << "IfThenSuspendElseSuspend: Successfully set up "
                               "branch execution";
            } catch (const Exception::Base& e) {
                KAI_TRACE_ERROR() << "IfThenSuspendElseSuspend: KAI exception: "
                                  << e.ToString();
            } catch (const std::exception& e) {
                KAI_TRACE_ERROR()
                    << "IfThenSuspendElseSuspend: std::exception: " << e.what();
            } catch (...) {
                KAI_TRACE_ERROR()
                    << "IfThenSuspendElseSuspend: Unknown exception";
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
                KAI_THROW_1(Base,
                            "Not enough values on stack for modulo operation");
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
                KAI_TRACE_ERROR()
                    << "Modulo operation failed to produce a result";
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
            } else if (value.IsType<int>()) {
                condition = ConstDeref<int>(value) != 0;
            } else if (value.IsType<float>() || value.IsType<double>()) {
                condition = ConstDeref<float>(value) != 0.0f;
            } else if (value.IsType<String>()) {
                // Consider empty string as false, non-empty as true
                condition = !ConstDeref<String>(value).empty();
            } else {
                // For object types, consider existence/validity as the
                // condition
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
            } else if (obj.IsType<List>()) {
                Push(New<int>(Deref<List>(obj).Size()));
            } else if (obj.IsType<Map>()) {
                Push(New<int>(Deref<Map>(obj).Size()));
            } else if (obj.IsType<String>()) {
                Push(New<int>(Deref<String>(obj).size()));
            } else {
                KAI_THROW_1(Base, "Size operation called on unsupported type");
            }
            break;
        }

        case Operation::Print: {
            // Print the top object from the stack and pop it
            if (!data_->Empty()) {
                Object obj = Pop();
                if (obj.Exists()) {
                    // Convert to string and print
                    StringStream stream;
                    obj.GetClass()->Insert(stream, obj.GetStorageBase());
                    std::cout << stream.ToString() << std::endl;
                }
            }
            break;
        }

        case Operation::ShellCommand: {
#ifdef ENABLE_SHELL_SYNTAX
            // Execute shell command and push result
            // Stack: ( command -- result )
            if (!data_->Empty()) {
                Object cmdObj = Pop();
                if (cmdObj.IsType<String>()) {
                    String command = ConstDeref<String>(cmdObj);

                    // Execute the command and capture output
                    FILE* pipe = popen(command.c_str(), "r");
                    if (pipe) {
                        char buffer[1024];
                        std::string result;

                        // Read command output
                        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                            result += buffer;
                        }

                        int status = pclose(pipe);

                        // Check if command execution failed
                        if (status != 0) {
                            KAI_TRACE()
                                << "Shell command exited with non-zero status: "
                                << status;
                        }

                        // Remove trailing newline if present
                        if (!result.empty() && result.back() == '\n') {
                            result.pop_back();
                        }

                        // Try to parse as a number first
                        bool isNumber = true;
                        bool isFloat = false;
                        bool hasDigits = false;

                        // Check if the result looks like a number
                        for (size_t i = 0; i < result.size(); ++i) {
                            char c = result[i];
                            if (i == 0 && (c == '-' || c == '+')) {
                                continue;  // Allow sign at beginning
                            }
                            if (c == '.' && !isFloat) {
                                isFloat = true;
                                continue;  // Allow one decimal point
                            }
                            if (!std::isdigit(c)) {
                                isNumber = false;
                                break;
                            }
                            hasDigits = true;
                        }

                        // Make sure we have at least one digit
                        isNumber = isNumber && hasDigits;

                        if (isNumber) {
                            try {
                                if (isFloat) {
                                    float value = std::stof(result);
                                    Push(New<float>(value));
                                } else {
                                    int value = std::stoi(result);
                                    Push(New<int>(value));
                                }
                            } catch (...) {
                                // If parsing fails, push as string
                                Push(New<String>(result));
                            }
                        } else {
                            // Push as string if not a number
                            Push(New<String>(result));
                        }
                    } else {
                        // Command execution failed
                        KAI_TRACE_ERROR()
                            << "ShellCommand: Failed to execute command: "
                            << command;
                        Push(New<String>(""));  // Push empty string on failure
                    }
                } else {
                    KAI_TRACE_ERROR()
                        << "ShellCommand: Expected string on stack";
                    Push(New<String>(""));  // Push empty string on error
                }
            } else {
                KAI_TRACE_ERROR() << "ShellCommand: Empty stack";
                Push(New<String>(""));  // Push empty string on error
            }
#else
            // Shell operations are disabled
            KAI_THROW_1(NotImplemented,
                        "Shell operations are disabled for security. Enable "
                        "with -DENABLE_SHELL_SYNTAX=ON");
#endif
            break;
        }

        case Operation::WhileLoop: {
            // ( condition body -- )
            // While condition is true, run body.
            KAI_TRACE() << "WhileLoop: Getting continuations from stack";
            try {
                // Check for valid data stack first
                if (!data_.Valid() || !data_.Exists()) {
                    KAI_TRACE_ERROR() << "WhileLoop: Invalid data stack";
                    break;
                }

                // Check if we have enough items on the stack
                if (data_->Size() < 2) {
                    KAI_TRACE_ERROR() << "WhileLoop: Not enough items on stack "
                                         "(need at least 2)";
                    break;
                }

                // Get the body continuation
                auto bodyCont = Pop();
                auto condCont = Pop();

                if (!condCont.IsType<Continuation>() ||
                    !bodyCont.IsType<Continuation>()) {
                    KAI_TRACE_ERROR() << "WhileLoop: Expected continuations";
                    break;
                }

                Pointer<Continuation> condition = condCont;
                Pointer<Continuation> body = bodyCont;

                // Execute while loop inline
                break_ = false;  // Reset break flag
                while (true) {
                    continue_ = false;  // Reset continue flag at loop start

                    // Evaluate condition using ExecuteContinuationInline
                    ExecuteContinuationInline(condition);

                    // Check condition result
                    if (data_->Empty() || !PopBool()) {
                        break;
                    }

                    // Execute body using ExecuteContinuationInline
                    ExecuteContinuationInline(body);

                    // Check for break after body execution
                    if (break_) {
                        break_ = false;  // Reset for next loop
                        break;           // Exit the while loop
                    }
                    // If continue_ is set, it just goes to next iteration
                }
            } catch (const Exception::Base& e) {
                KAI_TRACE_ERROR()
                    << "WhileLoop: KAI exception: " << e.ToString();
            } catch (const std::exception& e) {
                KAI_TRACE_ERROR() << "WhileLoop: std::exception: " << e.what();
            } catch (...) {
                KAI_TRACE_ERROR() << "WhileLoop: Unknown exception";
            }

            break;
        }

        case Operation::ForLoop: {
            KAI_TRACE() << "ForLoop: Starting execution";

            try {
                // Validate stack
                if (!data_.Valid() || !data_.Exists()) {
                    KAI_TRACE_ERROR() << "ForLoop: Invalid data stack";
                    break;
                }

                if (data_->Size() < 4) {
                    KAI_TRACE_ERROR()
                        << "ForLoop: Need at least 4 items on stack, have: "
                        << data_->Size();
                    break;
                }

                // Debug: print what's on the stack
                KAI_TRACE() << "ForLoop: Stack size: " << data_->Size();
                KAI_TRACE() << "ForLoop: Checking stack positions for "
                               "range-based syntax:";
                if (data_->Size() >= 4) {
                    for (int i = 0; i < 4; ++i) {
                        auto item = data_->At(i);
                        KAI_TRACE() << "  Position " << i << " (expecting "
                                    << (i == 0   ? "int accumulator"
                                        : i == 1 ? "int start"
                                        : i == 2 ? "int end"
                                                 : "Continuation body")
                                    << "): ";
                        if (item.GetClass()) {
                            KAI_TRACE()
                                << "    Found: " << item.GetClass()->GetName()
                                << " (type: " << item.GetTypeNumber().ToInt()
                                << ")";
                            if (item.IsType<int>()) {
                                KAI_TRACE()
                                    << "    value: " << ConstDeref<int>(item);
                            }
                        } else {
                            KAI_TRACE() << "    Found: <no class>";
                        }
                    }
                }

                // Check the stack to determine syntax type
                // Note: In Pi, items are pushed in order, so "0 1 5 { + } for"
                // results in: Bottom [0]: 0, [1]: 1, [2]: 5, [3]: { + }

                // SYNTAX 1: Range-based (Pi style)
                // Stack order from bottom to top: accumulator start end
                // body_continuation For "0 1 5 { + } for", the stack is: At(3)
                // = 0 (accumulator, bottom - can be any type) At(2) = 1 (start)
                // At(1) = 5 (end)
                // At(0) = { + } (body, top)
                if (data_->Size() >= 4 &&
                    data_->At(2).IsType<int>() &&  // start must be int
                    data_->At(1).IsType<int>() &&  // end must be int
                    data_->At(0)
                        .IsType<Continuation>()) {  // body must be continuation

                    KAI_TRACE() << "ForLoop: Range-based syntax detected";

                    // Pop in reverse order (top to bottom)
                    auto body = Pop();                   // Top: continuation
                    int end = ConstDeref<int>(Pop());    // end value
                    int start = ConstDeref<int>(Pop());  // start value
                    Object accumulator = Pop();          // Bottom: accumulator

                    if (!body.IsType<Continuation>()) {
                        KAI_TRACE_ERROR()
                            << "ForLoop: Body must be continuation";
                        Push(accumulator);
                        break;
                    }

                    Pointer<Continuation> bodyCont = body;

                    // Execute range loop
                    for (int i = start; i <= end; ++i) {
                        // Push accumulator and current value
                        Push(accumulator);
                        Push(New<int>(i));

                        if (traceLevel_ > 0) {
                            KAI_TRACE() << "ForLoop iteration " << i
                                        << ": accumulator=" << accumulator
                                        << ", current=" << i;
                        }

                        // Execute body inline
                        if (bodyCont->GetCode().Exists()) {
                            for (int j = 0; j < bodyCont->GetCode()->Size();
                                 ++j) {
                                if (break_ || continue_) break;
                                auto obj = bodyCont->GetCode()->At(j);
                                if (obj.Exists()) {
                                    Eval(obj);
                                }
                            }
                        }

                        // Handle control flow
                        if (break_) {
                            break_ = false;
                            break;
                        }

                        // Get new accumulator value
                        if (!data_->Empty()) {
                            accumulator = Pop();
                            if (traceLevel_ > 0) {
                                KAI_TRACE() << "ForLoop: New accumulator after "
                                               "iteration "
                                            << i << ": " << accumulator;
                            }
                        } else {
                            KAI_TRACE_ERROR()
                                << "ForLoop: Stack empty after body execution";
                        }
                    }

                    // Push final accumulator
                    Push(accumulator);
                }
                // SYNTAX 2: Traditional (C-style)
                // Stack: init_cont cond_cont incr_cont body_cont
                else {
                    KAI_TRACE() << "ForLoop: Traditional syntax detected";

                    auto body = Pop();
                    auto incr = Pop();
                    auto cond = Pop();
                    auto init = Pop();

                    // Validate all are continuations
                    if (!init.IsType<Continuation>() ||
                        !cond.IsType<Continuation>() ||
                        !incr.IsType<Continuation>() ||
                        !body.IsType<Continuation>()) {
                        KAI_TRACE_ERROR()
                            << "ForLoop: All 4 items must be continuations";
                        break;
                    }

                    Pointer<Continuation> initCont = init;
                    Pointer<Continuation> condCont = cond;
                    Pointer<Continuation> incrCont = incr;
                    Pointer<Continuation> bodyCont = body;

                    // Execute initialization
                    ExecuteContinuationInline(initCont);

                    // Main loop
                    break_ = false;
                    while (true) {
                        continue_ = false;

                        // Check condition
                        ExecuteContinuationInline(condCont);

                        if (data_->Empty() || !PopBool()) {
                            break;
                        }

                        // Execute body
                        ExecuteContinuationInline(bodyCont);

                        if (break_) {
                            break_ = false;
                            break;
                        }

                        // Execute increment (even with continue)
                        ExecuteContinuationInline(incrCont);
                    }
                }
            } catch (const Exception::Base& e) {
                KAI_TRACE_ERROR() << "ForLoop: " << e.ToString();
            } catch (const std::exception& e) {
                KAI_TRACE_ERROR() << "ForLoop: " << e.what();
            }

            break;
        }

        case Operation::DoLoop: {
            // ( body cond -- )
            // Do-while loop: execute body first, then check condition
            KAI_TRACE() << "DoLoop: Getting continuations from stack";
            try {
                // Check for valid data stack first
                if (!data_.Valid() || !data_.Exists()) {
                    KAI_TRACE_ERROR() << "DoLoop: Invalid data stack";
                    break;
                }

                // Check if we have enough items on the stack
                if (data_->Size() < 2) {
                    KAI_TRACE_ERROR() << "DoLoop: Not enough items on stack "
                                         "(need at least 2)";
                    break;
                }

                // Get continuations
                auto condCont = Pop();
                auto bodyCont = Pop();

                if (!condCont.IsType<Continuation>() ||
                    !bodyCont.IsType<Continuation>()) {
                    KAI_TRACE_ERROR() << "DoLoop: Expected continuations";
                    break;
                }

                Pointer<Continuation> condition = condCont;
                Pointer<Continuation> body = bodyCont;

                // Execute do-while loop inline
                break_ = false;  // Reset break flag
                do {
                    continue_ = false;  // Reset continue flag at loop start

                    // Execute body using ExecuteContinuationInline
                    ExecuteContinuationInline(body);

                    // Check for break after body execution
                    if (break_) {
                        break_ = false;  // Reset for next loop
                        break;           // Exit the do-while loop
                    }

                    // Evaluate condition (even if continue was hit) using
                    // ExecuteContinuationInline
                    ExecuteContinuationInline(condition);

                    // Check condition result
                } while (!data_->Empty() && PopBool());
            } catch (const Exception::Base& e) {
                KAI_TRACE_ERROR() << "DoLoop: KAI exception: " << e.ToString();
            } catch (const std::exception& e) {
                KAI_TRACE_ERROR() << "DoLoop: std::exception: " << e.what();
            } catch (...) {
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
                    KAI_TRACE_ERROR()
                        << "Jump: Empty stack, expected jump target";
                    break;
                }

                // Get the jump target
                Object jumpTarget = data_->Top();
                data_->Pop();

                // Verify that the jump target exists
                if (!jumpTarget.Exists()) {
                    KAI_TRACE_ERROR()
                        << "Jump: Invalid jump target (null object)";
                    break;
                }

                // If the jump target is a continuation, extract the first
                // element as the label
                if (jumpTarget.IsType<Continuation>()) {
                    Pointer<Continuation> jumpCont = jumpTarget;
                    if (jumpCont->GetCode().Exists() &&
                        jumpCont->GetCode()->Size() > 0) {
                        Object labelObj = jumpCont->GetCode()->At(0);

                        // If the first element is a label, find its target
                        if (labelObj.IsType<Label>()) {
                            Label targetLabel = ConstDeref<Label>(labelObj);
                            KAI_TRACE() << "Jump: Jumping to label "
                                        << targetLabel.ToString();

                            // Check if we have a valid target point in the code
                            // The target should be in the code of the current
                            // continuation
                            if (continuation_.Exists()) {
                                // Get a pointer to the current continuation
                                auto cont = continuation_.GetObject();
                                if (cont.IsType<Continuation>()) {
                                    Pointer<Continuation> currentCont = cont;
                                    Pointer<Array> code =
                                        currentCont->GetCode();

                                    // Search for the label in the code
                                    if (code.Exists()) {
                                        // Find the position of the label in the
                                        // current code
                                        int pos = -1;
                                        for (int i = 0; i < code->Size(); ++i) {
                                            if (code->At(i).IsType<Label>() &&
                                                ConstDeref<Label>(code->At(
                                                    i)) == targetLabel) {
                                                pos = i;
                                                break;
                                            }
                                        }

                                        if (pos >= 0) {
                                            // We found the label, set the
                                            // instruction pointer to that
                                            // position
                                            currentCont->SetInstructionPointer(
                                                pos);
                                            KAI_TRACE() << "Jump: Found label "
                                                           "at position "
                                                        << pos;
                                        } else {
                                            KAI_TRACE_ERROR()
                                                << "Jump: Label not found in "
                                                   "current code";
                                        }
                                    } else {
                                        KAI_TRACE_ERROR()
                                            << "Jump: Current continuation has "
                                               "no code";
                                    }
                                } else {
                                    KAI_TRACE_ERROR()
                                        << "Jump: Current continuation is not "
                                           "a Continuation";
                                }
                            } else {
                                KAI_TRACE_ERROR()
                                    << "Jump: No valid current continuation";
                            }
                        } else {
                            KAI_TRACE_ERROR() << "Jump: First element in jump "
                                                 "target is not a label";
                        }
                    } else {
                        KAI_TRACE_ERROR()
                            << "Jump: Empty jump target continuation";
                    }
                } else {
                    KAI_TRACE_ERROR()
                        << "Jump: Jump target is not a continuation";
                }
            } catch (const Exception::Base& e) {
                KAI_TRACE_ERROR() << "Jump: KAI exception: " << e.ToString();
            } catch (const std::exception& e) {
                KAI_TRACE_ERROR() << "Jump: std::exception: " << e.what();
            } catch (...) {
                KAI_TRACE_ERROR() << "Jump: Unknown exception";
            }

            break;
        }

        case Operation::IfFalseJump: {
            // Conditional jump to a label if the top of the stack is false
            // Stack: ( condition target -- )
            // where condition is a boolean and target is a continuation
            // containing a label
            try {
                // Check for valid data stack first
                if (!data_.Valid() || !data_.Exists()) {
                    KAI_TRACE_ERROR() << "IfFalseJump: Invalid data stack";
                    break;
                }

                // Check if we have enough items on the stack
                if (data_->Size() < 1) {
                    KAI_TRACE_ERROR() << "IfFalseJump: Not enough items on "
                                         "stack (need jump target)";
                    break;
                }

                // Get the jump target
                Object jumpTarget = data_->Top();
                data_->Pop();

                // Check if we have a condition value
                if (data_->Empty()) {
                    KAI_TRACE_ERROR()
                        << "IfFalseJump: No condition value on stack";
                    break;
                }

                // Get the condition value
                bool condition = PopBool();

                // If the condition is true, we don't jump
                if (condition) {
                    KAI_TRACE()
                        << "IfFalseJump: Condition is true, not jumping";
                    break;
                }

                // Condition is false, perform the jump using the same logic as
                // Jump operation
                KAI_TRACE() << "IfFalseJump: Condition is false, jumping";

                // Verify that the jump target exists
                if (!jumpTarget.Exists()) {
                    KAI_TRACE_ERROR()
                        << "IfFalseJump: Invalid jump target (null object)";
                    break;
                }

                // If the jump target is a continuation, extract the first
                // element as the label
                if (jumpTarget.IsType<Continuation>()) {
                    Pointer<Continuation> jumpCont = jumpTarget;
                    if (jumpCont->GetCode().Exists() &&
                        jumpCont->GetCode()->Size() > 0) {
                        Object labelObj = jumpCont->GetCode()->At(0);

                        // If the first element is a label, find its target
                        if (labelObj.IsType<Label>()) {
                            Label targetLabel = ConstDeref<Label>(labelObj);
                            KAI_TRACE() << "IfFalseJump: Jumping to label "
                                        << targetLabel.ToString();

                            // Check if we have a valid target point in the code
                            // The target should be in the code of the current
                            // continuation
                            if (continuation_.Exists()) {
                                // Get a pointer to the current continuation
                                auto cont = continuation_.GetObject();
                                if (cont.IsType<Continuation>()) {
                                    Pointer<Continuation> currentCont = cont;
                                    Pointer<Array> code =
                                        currentCont->GetCode();

                                    // Search for the label in the code
                                    if (code.Exists()) {
                                        // Find the position of the label in the
                                        // current code
                                        int pos = -1;
                                        for (int i = 0; i < code->Size(); ++i) {
                                            if (code->At(i).IsType<Label>() &&
                                                ConstDeref<Label>(code->At(
                                                    i)) == targetLabel) {
                                                pos = i;
                                                break;
                                            }
                                        }

                                        if (pos >= 0) {
                                            // We found the label, set the
                                            // instruction pointer to that
                                            // position
                                            currentCont->SetInstructionPointer(
                                                pos);
                                            KAI_TRACE() << "IfFalseJump: Found "
                                                           "label at position "
                                                        << pos;
                                        } else {
                                            KAI_TRACE_ERROR()
                                                << "IfFalseJump: Label not "
                                                   "found in current code";
                                        }
                                    } else {
                                        KAI_TRACE_ERROR()
                                            << "IfFalseJump: Current "
                                               "continuation has no code";
                                    }
                                } else {
                                    KAI_TRACE_ERROR()
                                        << "IfFalseJump: Current continuation "
                                           "is not a Continuation";
                                }
                            } else {
                                KAI_TRACE_ERROR() << "IfFalseJump: No valid "
                                                     "current continuation";
                            }
                        } else {
                            KAI_TRACE_ERROR()
                                << "IfFalseJump: First element in jump target "
                                   "is not a label";
                        }
                    } else {
                        KAI_TRACE_ERROR()
                            << "IfFalseJump: Empty jump target continuation";
                    }
                } else {
                    KAI_TRACE_ERROR()
                        << "IfFalseJump: Jump target is not a continuation";
                }
            } catch (const Exception::Base& e) {
                KAI_TRACE_ERROR()
                    << "IfFalseJump: KAI exception: " << e.ToString();
            } catch (const std::exception& e) {
                KAI_TRACE_ERROR()
                    << "IfFalseJump: std::exception: " << e.what();
            } catch (...) {
                KAI_TRACE_ERROR() << "IfFalseJump: Unknown exception";
            }

            break;
        }

        case Operation::UnnnamedOp: {
            // UnnnamedOp is a placeholder for operations that don't have a
            // specific implementation In most cases, we can just ignore it
            // without causing an error
            KAI_TRACE() << "Ignoring UnnnamedOp operation";
            break;
        }

        case Operation::Index: {
            // ( array index -- element )
            // Get element at index from array/list
            if (data_->Size() < 2) {
                KAI_TRACE_ERROR() << "Index: Not enough items on stack";
                break;
            }

            Object index = Pop();
            Object container = Pop();

            if (!container.Exists()) {
                KAI_TRACE_ERROR() << "Index: Container is null";
                Push(Object());
                break;
            }

            if (container.IsType<Array>()) {
                if (!index.IsType<int>()) {
                    KAI_TRACE_ERROR()
                        << "Index: Array index must be an integer";
                    Push(Object());
                    break;
                }

                int idx = ConstDeref<int>(index);
                Pointer<Array> arr = container;
                if (idx < 0 || idx >= arr->Size()) {
                    KAI_TRACE_ERROR()
                        << "Index: Array index out of bounds: " << idx;
                    Push(Object());
                    break;
                }
                Push(arr->At(idx));
            } else if (container.IsType<Map>()) {
                // For maps, index can be any type that can be used as a key
                Pointer<Map> map = container;

                // Convert Pathname to String for compatibility
                Object searchKey = index;
                if (index.IsType<Pathname>()) {
                    String pathStr = ConstDeref<Pathname>(index).ToString();
                    // Remove the leading quote if present
                    if (pathStr.StartsWith("'")) {
                        pathStr = String(pathStr.c_str() + 1);
                    }
                    searchKey = New<String>(pathStr);
                }

                auto it = map->Find(searchKey);
                if (it != map->End()) {
                    Push(it->second);
                } else {
                    KAI_TRACE_ERROR() << "Index: Map key '"
                                      << searchKey.ToString() << "' not found";
                    Push(Object());
                }
            } else if (container.IsType<String>()) {
                // String indexing - return character at index
                if (!index.IsType<int>()) {
                    KAI_TRACE_ERROR()
                        << "Index: String index must be an integer";
                    Push(Object());
                    break;
                }

                int idx = ConstDeref<int>(index);
                String str = ConstDeref<String>(container);
                if (idx < 0 || idx >= static_cast<int>(str.size())) {
                    KAI_TRACE_ERROR()
                        << "Index: String index out of bounds: " << idx;
                    Push(Object());
                    break;
                }
                // Return single character as a String
                Push(New<String>(String(1, str[idx])));
            } else {
                KAI_TRACE_ERROR() << "Index: Container type "
                                  << container.GetTypeNumber().ToString()
                                  << " not supported for indexing";
                Push(Object());
            }
            break;
        }

        case Operation::SetChild: {
            // ( array index value -- array )
            // Set element at index in array/list
            if (data_->Size() < 3) {
                KAI_TRACE_ERROR() << "SetChild: Not enough items on stack";
                break;
            }

            Object value = Pop();
            Object index = Pop();
            Object container = Pop();

            if (!container.Exists()) {
                KAI_TRACE_ERROR() << "SetChild: Container is null";
                Push(container);
                break;
            }

            if (container.IsType<Array>()) {
                if (!index.IsType<int>()) {
                    KAI_TRACE_ERROR()
                        << "SetChild: Array index must be an integer";
                    Push(container);
                    break;
                }

                int idx = ConstDeref<int>(index);
                Pointer<Array> arr = container;
                if (idx < 0 || idx >= arr->Size()) {
                    KAI_TRACE_ERROR()
                        << "SetChild: Array index out of bounds: " << idx;
                    Push(container);
                    break;
                }
                // Use RefAt to get a reference and assign the value
                arr->RefAt(idx) = value;
                Push(container);
            } else if (container.IsType<Map>()) {
                // For maps, index can be any type that can be used as a key
                Pointer<Map> map = container;

                // Convert Pathname to String for compatibility
                Object mapKey = index;
                if (index.IsType<Pathname>()) {
                    String pathStr = ConstDeref<Pathname>(index).ToString();
                    // Remove the leading quote if present
                    if (pathStr.StartsWith("'")) {
                        pathStr = String(pathStr.c_str() + 1);
                    }
                    mapKey = New<String>(pathStr);
                }

                map->Insert(mapKey, value);
                Push(container);
            } else {
                KAI_TRACE_ERROR() << "SetChild: Container type "
                                  << container.GetTypeNumber().ToString()
                                  << " not supported for indexing";
                Push(container);
            }
            break;
        }

        case Operation::Min: {
            Object B = Pop();
            Object A = Pop();

            // Use type traits via PerformBinaryOp for all types
            Object result = PerformBinaryOp(A, B, Operation::Min);
            Push(result);
            break;
        }

        case Operation::Max: {
            Object B = Pop();
            Object A = Pop();

            // Use type traits via PerformBinaryOp for all types
            Object result = PerformBinaryOp(A, B, Operation::Max);
            Push(result);
            break;
        }

        case Operation::Return: {
            KAI_TRACE() << "Return operation";

            // Get the return value from the stack (if any)
            Object returnValue;
            if (!GetDataStack()->Empty()) {
                returnValue = Pop();
            }

            // Signal that we want to return from this continuation
            break_ = true;

            // Push the return value back on the stack for the caller
            if (returnValue.Exists()) {
                Push(returnValue);
            }

            // For function returns, we need to resume the suspended
            // continuation The Suspend operation pushed the caller's
            // continuation onto the context stack So when we return,
            // NextContinuation will pop it and resume execution

            break;
        }

        case Operation::None: {
            // Push a null/none object onto the stack
            Push(Object());
            break;
        }

        case Operation::Self: {
            // Self operation: print the top of the stack without popping
            auto stack = GetDataStack();
            if (!stack->Empty()) {
                Object top = stack->Top();
                std::cout << top.ToString() << std::endl;
            }
            break;
        }

        case Operation::ToStringOp: {
            // Convert top of stack to string
            if (!data_->Empty()) {
                Object obj = Pop();
                if (obj.Exists()) {
                    StringStream stream;
                    obj.GetClass()->Insert(stream, obj.GetStorageBase());
                    Push(New<String>(stream.ToString()));
                } else {
                    Push(New<String>(""));
                }
            } else {
                KAI_TRACE_ERROR() << "ToStringOp: Empty stack";
                Push(New<String>(""));
            }
            break;
        }

        case Operation::ForEach: {
            // ForEach operation
            // Stack: ( collection function -- result_array )
            KAI_TRACE() << "ForEach: Starting foreach operation";

            if (data_->Size() < 2) {
                KAI_TRACE_ERROR() << "ForEach: Not enough arguments on stack";
                KAI_THROW_1(Base, "ForEach requires collection and function");
            }

            // Pop function and collection
            Object function = Pop();
            Object collection = Pop();

            if (!function.Exists() || !collection.Exists()) {
                KAI_TRACE_ERROR() << "ForEach: Invalid arguments";
                KAI_THROW_1(Base,
                            "ForEach requires valid collection and function");
            }

            // Don't create a result array - foreach is for side effects

            // Handle different collection types
            if (collection.IsType<Array>()) {
                auto& arr = Deref<Array>(collection);
                KAI_TRACE() << "ForEach: Processing array with " << arr.Size()
                            << " elements";

                for (int i = 0; i < arr.Size(); ++i) {
                    // Push the current element
                    Push(arr.At(i));

                    // Execute the function
                    if (function.IsType<Continuation>()) {
                        // Cast to continuation pointer
                        Pointer<Continuation> cont = function;
                        ExecuteContinuationInline(cont);
                    } else {
                        // For other callable types, use Continue
                        Continue(function);
                    }

                    // Check for break
                    if (break_) {
                        break_ = false;
                        break;
                    }

                    // Check for continue
                    if (continue_) {
                        continue_ = false;
                        continue;
                    }

                    // Pop any result left by the body (we don't collect it)
                    if (!data_->Empty()) {
                        Pop();
                    }
                }
            } else if (collection.IsType<List>()) {
                auto& list = Deref<List>(collection);
                KAI_TRACE() << "ForEach: Processing list with " << list.Size()
                            << " elements";

                for (auto it = list.Begin(); it != list.End(); ++it) {
                    // Push the current element
                    Push(*it);

                    // Execute the function
                    if (function.IsType<Continuation>()) {
                        Pointer<Continuation> cont = function;
                        ExecuteContinuationInline(cont);
                    } else {
                        Continue(function);
                    }

                    // Check for break
                    if (break_) {
                        break_ = false;
                        break;
                    }

                    // Check for continue
                    if (continue_) {
                        continue_ = false;
                        continue;
                    }

                    // Pop any result left by the body (we don't collect it)
                    if (!data_->Empty()) {
                        Pop();
                    }
                }
            } else if (collection.IsType<String>()) {
                auto& str = Deref<String>(collection);
                KAI_TRACE() << "ForEach: Processing string with " << str.size()
                            << " characters";

                for (char ch : str) {
                    // Push the current character as a string
                    Push(New<String>(std::string(1, ch)));

                    // Execute the function
                    if (function.IsType<Continuation>()) {
                        Pointer<Continuation> cont = function;
                        ExecuteContinuationInline(cont);
                    } else {
                        Continue(function);
                    }

                    // Check for break
                    if (break_) {
                        break_ = false;
                        break;
                    }

                    // Check for continue
                    if (continue_) {
                        continue_ = false;
                        continue;
                    }

                    // Pop any result left by the body (we don't collect it)
                    if (!data_->Empty()) {
                        Pop();
                    }
                }
            } else if (collection.IsType<Map>()) {
                auto& map = Deref<Map>(collection);
                KAI_TRACE() << "ForEach: Processing map with " << map.Size()
                            << " entries";

                for (auto it = map.Begin(); it != map.End(); ++it) {
                    // Push key-value pair as an array
                    auto pair = New<Array>();
                    pair->Append(it->first);
                    pair->Append(it->second);
                    Push(pair);

                    // Execute the function
                    if (function.IsType<Continuation>()) {
                        Pointer<Continuation> cont = function;
                        ExecuteContinuationInline(cont);
                    } else {
                        Continue(function);
                    }

                    // Check for break
                    if (break_) {
                        break_ = false;
                        break;
                    }

                    // Check for continue
                    if (continue_) {
                        continue_ = false;
                        continue;
                    }

                    // Pop any result left by the body (we don't collect it)
                    if (!data_->Empty()) {
                        Pop();
                    }
                }
            } else {
                KAI_TRACE_ERROR() << "ForEach: Unsupported collection type: "
                                  << collection.GetTypeNumber().ToString();
                KAI_THROW_1(Base,
                            "ForEach requires array, list, string, or map");
            }

            // ForEach doesn't push anything - it's for side effects only
            KAI_TRACE() << "ForEach: Completed";
            break;
        }

        default: {
            // Provide a default implementation for unimplemented operations
            KAI_TRACE_ERROR()
                << "Unimplemented operation: " << Operation::ToString(op);
            KAI_THROW_1(Base, "Unimplemented operation");
            break;
        }
    }
}

// Helper method to execute a continuation's code inline
void Executor::ExecuteContinuationInline(Pointer<Continuation> cont) {
    if (cont.Exists() && cont->GetCode().Exists()) {
        // Create a temporary sub-continuation for this inline execution
        // This allows proper handling of Suspend operations
        auto savedCont = continuation_;
        auto savedIndex =
            savedCont.Exists() ? ConstDeref<int>(savedCont->index) : 0;

        // Set up the inline continuation
        continuation_ = cont;
        if (!continuation_->index.Exists()) {
            continuation_->index = continuation_->New<int>(0);
        } else {
            *continuation_->index = 0;
        }

        // Execute the continuation
        try {
            while (continuation_.Exists() &&
                   ConstDeref<int>(continuation_->index) <
                       continuation_->GetCode()->Size()) {
                if (break_ || continue_) break;

                int index = ConstDeref<int>(continuation_->index);
                auto obj = continuation_->GetCode()->At(index);

                // Advance the index first
                *continuation_->index = index + 1;

                if (obj.Exists()) {
                    Eval(obj);
                }
            }
        } catch (...) {
            // Restore original continuation on error
            continuation_ = savedCont;
            if (savedCont.Exists() && savedCont->index.Exists()) {
                *savedCont->index = savedIndex;
            }
            throw;
        }

        // Restore the original continuation
        continuation_ = savedCont;
        if (savedCont.Exists() && savedCont->index.Exists()) {
            *savedCont->index = savedIndex;
        }
    }
}

KAI_END
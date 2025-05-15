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
        
        // For all languages, use a direct evaluation approach to solve the Type Mismatch issues
        // For null or empty continuations, nothing to do
        if (!cont.Exists() || !cont->GetCode().Exists() || cont->GetCode()->Size() == 0) {
            return;
        }
        
        // Get the data stack for easier access
        Value<Stack> dataStack = executor->GetDataStack();
        
        // If we have special handling flag set, directly evaluate the continuation without creating surrogate continuations
        if (cont->GetSpecialHandling()) {
            KAI_TRACE() << "Using special handling for continuation with direct evaluation";
            
            // Check for nested continuations (most Pi blocks come as a single continuation object in an array)
            if (cont->GetCode()->Size() == 1) {
                Object firstItem = cont->GetCode()->At(0);
                
                // Check if the first item is a continuation (which contains the actual code)
                if (firstItem.IsType<Continuation>()) {
                    KAI_TRACE_1("Found inner continuation with special handling, executing it directly");
                    Continuation& innerCont = Deref<Continuation>(firstItem);
                    
                    // Create a temporary continuation to execute
                    Pointer<Continuation> innerContPtr = reg_->New<Continuation>();
                    innerContPtr->SetCode(innerCont.GetCode());
                    innerContPtr->SetSpecialHandling(true); // Ensure we keep the special handling flag
                    
                    // Execute the inner continuation directly
                    Execute(innerContPtr);
                    return;
                }
            }
            
            // Process each operation directly using proper type handling
            for (int i = 0; i < cont->GetCode()->Size(); i++) {
                Object item = cont->GetCode()->At(i);
                
                // Skip the ContinuationBegin and ContinuationEnd operations
                if (item.IsType<Operation>()) {
                    Operation::Type opType = Deref<Operation>(item).GetTypeNumber();
                    if (opType == Operation::ContinuationBegin || opType == Operation::ContinuationEnd) {
                        KAI_TRACE_1("Skipping continuation marker");
                        continue;
                    }
                }
                
                // For basic operations, handle them directly to ensure type preservation
                if (item.IsType<Operation>()) {
                    Operation::Type opType = Deref<Operation>(item).GetTypeNumber();
                    
                    // Handle all arithmetic operations
                    if (opType == Operation::Plus || opType == Operation::Minus || 
                        opType == Operation::Multiply || opType == Operation::Divide || 
                        opType == Operation::Modulo) {
                        if (dataStack->Size() >= 2) {
                            Object b = dataStack->Pop();
                            Object a = dataStack->Pop();
                            
                            // Use PerformBinaryOp for standard binary operations
                            Object result = executor->PerformBinaryOp(a, b, opType);
                            dataStack->Push(result);
                            continue;
                        }
                    }
                    
                    // Handle all comparison operations
                    if (opType == Operation::Equiv || opType == Operation::NotEquiv || 
                        opType == Operation::Less || opType == Operation::Greater || 
                        opType == Operation::LessOrEquiv || opType == Operation::GreaterOrEquiv) {
                        if (dataStack->Size() >= 2) {
                            Object b = dataStack->Pop();
                            Object a = dataStack->Pop();
                            
                            // Use PerformBinaryOp for comparison operations
                            Object result = executor->PerformBinaryOp(a, b, opType);
                            dataStack->Push(result);
                            continue;
                        }
                    }
                    
                    // Handle logical operations
                    if (opType == Operation::LogicalAnd || opType == Operation::LogicalOr || 
                        opType == Operation::LogicalXor) {
                        if (dataStack->Size() >= 2) {
                            Object b = dataStack->Pop();
                            Object a = dataStack->Pop();
                            
                            // Use PerformBinaryOp for logical operations
                            Object result = executor->PerformBinaryOp(a, b, opType);
                            dataStack->Push(result);
                            continue;
                        }
                    }
                    
                    // Handle all stack operations directly
                    if (opType == Operation::Dup) {
                        if (dataStack->Size() > 0) {
                            dataStack->Push(dataStack->Top());
                            continue;
                        }
                    }
                    else if (opType == Operation::Swap) {
                        if (dataStack->Size() >= 2) {
                            Object b = dataStack->Pop();
                            Object a = dataStack->Pop();
                            dataStack->Push(b);
                            dataStack->Push(a);
                            continue;
                        }
                    }
                    else if (opType == Operation::Drop) {
                        if (dataStack->Size() >= 1) {
                            dataStack->Pop();
                            continue;
                        }
                    }
                    else if (opType == Operation::Over) {
                        if (dataStack->Size() >= 2) {
                            Object b = dataStack->Pop();
                            Object a = dataStack->Pop();
                            dataStack->Push(a);
                            dataStack->Push(b);
                            dataStack->Push(a);
                            continue;
                        }
                    }
                    else if (opType == Operation::Store || opType == Operation::Replace) {
                        if (dataStack->Size() >= 2) {
                            Object value = dataStack->Pop();
                            Object name = dataStack->Pop();
                            
                            // Handle different name types
                            if (name.IsType<Label>()) {
                                Label& label = Deref<Label>(name);
                                
                                // Store in scope
                                tree.GetScope().Set(label, value);
                            }
                            continue;
                        }
                    }
                    else if (opType == Operation::Retreive) {
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
                                continue;
                            }
                        }
                    }
                }
                
                // For other items, use regular evaluation
                executor->Eval(item);
            }
            
            return;
        }
        
        // For regular continuations (without special handling), execute them normally
        // but extract primitive result types after execution
        executor->Continue(cont);
        
        // After execution, always try to extract primitive values from continuations
        if (dataStack->Size() > 0) {
            Object result = dataStack->Top();
            
            // Check if we need to unwrap the value
            if (result.IsType<Continuation>()) {
                // Use the enhanced UnwrapValue method to extract primitive types
                Object unwrapped = executor->UnwrapValue(result);
                
                // If unwrapping produced a different value, replace the stack top
                if (unwrapped != result) {
                    if (unwrapped.Exists()) {
                        KAI_TRACE() << "Unwrapped continuation to primitive type: " 
                                  << unwrapped.GetClass()->GetName();
                    } else {
                        KAI_TRACE() << "Unwrapped continuation to null object";
                    }
                    
                    // Replace the continuation with the unwrapped value
                    dataStack->Pop(); // Remove the continuation
                    dataStack->Push(unwrapped); // Push the unwrapped value
                }
            }
        }
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
    KAI_TRACE() << "Executing text: " << text << " (with specialHandling=true)";
    
    // Mark the continuation for special handling for all languages
    // This ensures proper type handling for all operations
    cont->SetSpecialHandling(true);
    
    // Execute the continuation
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
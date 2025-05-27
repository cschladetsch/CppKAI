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
        auto result = alloc->Allocate<Registry>(alloc);
        if (!result.has_value()) {
            KAI_TRACE_ERROR() << "Could not allocate Registry";
            return;
        }
        reg_ = result.value();

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
        // Extra defensive check for necessary objects
        if (!executor.Exists()) {
            KAI_TRACE_ERROR() << "Execute: Null executor - skipping execution";
            return;
        }

        if (!executor->GetDataStack().Exists()) {
            KAI_TRACE_ERROR()
                << "Execute: Null data stack - skipping execution";
            return;
        }

        // Check for null continuation
        if (!cont.Exists()) {
            KAI_TRACE() << "Execute: Null continuation - skipping execution";
            return;
        }

        // Check if the continuation has valid code
        if (!cont->GetCode().Exists()) {
            KAI_TRACE()
                << "Execute: Continuation has no code - skipping execution";
            return;
        }

        // Debug the continuation code to help with diagnosing any issues
        KAI_TRACE_1(cont->GetCode()->Size())
            << "Executing continuation with size";

        // For null or empty continuations, nothing to do
        if (cont->GetCode()->Size() == 0) {
            KAI_TRACE() << "Execute: Continuation has empty code array - "
                           "skipping execution";
            return;
        }

        // Set the scope for the continuation if possible
        if (executor->GetTree() != nullptr) {
            cont->SetScope(executor->GetTree()->GetScope());
        }

        // Option 1: Execute the continuation using the standard executor
        if (!cont.Exists()) {
            KAI_TRACE_ERROR()
                << "Execute: Continuation is invalid - skipping execution";
            return;
        }

        // Let exceptions propagate so that Process can catch them
        // Use ContinueOnly to execute this continuation without
        // saving/restoring state
        executor->ContinueOnly(cont);
        KAI_TRACE() << "Execute: Continue returned, checking executor state";

        // After execution, process the stack to ensure proper type extraction
        Value<Stack> dataStack = executor->GetDataStack();

        // Check if we have a valid stack before processing
        if (!dataStack.Valid() || !dataStack.Exists()) {
            KAI_TRACE_WARN() << "Execute: Invalid data stack after execution";
            return;
        }

        KAI_TRACE() << "Execute: Stack size after execution: "
                    << dataStack->Size();

        // Process each stack item to extract primitive values from
        // continuations
        int stackSize = dataStack->Size();
        for (int i = 0; i < stackSize; i++) {
            // Get the object at the current position (from the bottom)
            // We want to preserve the original stack order
            // Removed unused variable to avoid null object access
            // We no longer automatically unwrap continuations here
            // Continuations are preserved by design for blocks and Pi {}
            // constructs Test code should use UnwrapStackValues() from
            // TestLangCommon if needed
        }

        // The continuation might have finished, which is normal
        // Don't access continuation properties after execution completes
    }
    KAI_CATCH(Exception::Base, E) {
        KAI_TRACE_ERROR_1(E);
        // Only re-throw assertion failures and similar errors that should be
        // visible to Process
        if (E.ToString().find("Assertion failed") != std::string::npos) {
            throw;
        }
        // For debugging: log stack state when exception occurs
        KAI_TRACE() << "Exception occurred. Stack state:";
        if (executor.Exists() && executor->GetDataStack().Exists()) {
            KAI_TRACE() << "  Stack size: " << executor->GetDataStack()->Size();
        }
    }
    KAI_CATCH(exception, E) {
        KAI_TRACE_ERROR_2("StdException: ", E.what());
        // Don't re-throw standard exceptions unless they're assertion-related
    }
    KAI_CATCH_ALL() {
        KAI_TRACE_ERROR_1("UnknownException");
        // Don't re-throw unknown exceptions
    }
}

void Console::Execute(String const &text, Structure st) {
    // Translate the text into a continuation
    Pointer<Continuation> cont = compiler->Translate(text.c_str(), st);
    if (!cont.Exists()) {
        KAI_TRACE_WARN() << "Translation of '" << text
                         << "' yielded invalid continuation";
        return;
    }

    // Log what we're about to execute for debugging purposes
    KAI_TRACE() << "Executing text: " << text;

    // Log the continuation details
    if (cont->GetCode().Exists()) {
        KAI_TRACE() << "Continuation code size: " << cont->GetCode()->Size();
        for (int i = 0; i < cont->GetCode()->Size(); ++i) {
            auto obj = cont->GetCode()->At(i);
            KAI_TRACE() << "  Code[" << i << "]: " << obj.ToString()
                        << " (type: "
                        << (obj.GetClass()
                                ? obj.GetClass()->GetName().ToString()
                                : "null")
                        << ")";
        }
    }

    // Set the scope on the continuation (important for Store operations)
    cont->SetScope(tree.GetScope());

    // Execute the continuation - let exceptions propagate to Process
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
    // Validate inputs first
    if (fileName == nullptr || strlen(fileName) == 0) {
        KAI_TRACE_ERROR() << "ExecuteFile: Null or empty filename";
        return false;
    }

    if (!compiler.Exists()) {
        KAI_TRACE_ERROR() << "ExecuteFile: Null compiler";
        return false;
    }

    // Compile the file
    Pointer<Continuation> c =
        compiler->CompileFile(fileName, Structure::Program);

    if (!c.Exists()) {
        KAI_TRACE_ERROR() << "ExecuteFile: Failed to compile " << fileName;
        return false;
    }

    // Execute the continuation using our improved method
    // This is safer than directly calling executor->Continue
    Execute(c);
    return true;
}

KAI_END

// EOF
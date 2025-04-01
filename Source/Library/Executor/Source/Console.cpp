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

Console::~Console() { alloc->DeAllocate(_reg); }

void Console::Create() {
    try {
        _reg = alloc->Allocate<Registry>(alloc);

        RegisterTypes();

        executor = _reg->New<Executor>();
        compiler = _reg->New<Compiler>();

        executor.SetManaged(false);
        compiler.SetManaged(false);

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
        const ClassBase *K = _reg->GetClass(N);
        if (K == 0) continue;
        types.Set(K->GetName(), _reg->New(K));
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
    Object root = _reg->New<void>();
    Object types = _reg->New<void>();
    Object sys = _reg->New<void>();
    Object bin = _reg->New<void>();
    Object home = _reg->New<void>();

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
    _reg->SetTree(tree);

    root.Set("Home", home);
    tree.SetScope(Pathname("/Home"));

    ExposeTypesToTree(types);
}

void Console::Execute(Pointer<Continuation> cont) {
    KAI_TRY {
        if (!cont->HasScope()) cont->SetScope(tree.GetRoot());

        executor->Continue(cont);
    }
    KAI_CATCH(Exception::Base, E) { KAI_TRACE_ERROR_1(E); }
    KAI_CATCH(exception, E) { KAI_TRACE_ERROR_2("StdException: ", E.what()); }
    KAI_CATCH_ALL() { KAI_TRACE_ERROR_1("UnknownException"); }
}

void Console::Execute(String const &text, Structure st) {
    Pointer<Continuation> cont = compiler->Translate(text.c_str(), st);
    if (!cont.Exists()) return;

    cont->SetScope(executor->GetTree()->GetScope());
    executor->Continue(cont);
}

String Console::Process(const String &text) {
    StringStream result;
    KAI_TRY {
        // Use direct ANSI codes for color (bold is already set)
        cout << "\033[31m"; // Red color
        auto cont = compiler->Translate(text.c_str());
        if (cont.Exists()) {
            cont->SetScope(tree.GetScope());
            // Switch to gray for execution
            cout << "\033[37m"; // Gray color
            Execute(cont);
        }

        return "";
    }
    KAI_CATCH(Exception::Base, E) {
        result << "Exception: " << E.ToString() << "\n";
    }
    KAI_CATCH(exception, E) { result << "StdException: " << E.what() << "\n"; }
    KAI_CATCH_ALL() {
        result << "UnknownException: "
               << "\n";
    }
    return result.ToString();
}

void Console::WritePrompt(ostream &out) const {
    const auto path = GetFullname(GetTree().GetScope());
    auto pathName = path.ToString();
    
    // Use direct ANSI escape codes for consistency
    // Bold is already set from the main loop, just need colors
    out << "\033[32m"; // Green for language
    out << ToString(static_cast<Language>(compiler->GetLanguage())) << " ";
    out << "\033[33m"; // Yellow for path
    out << pathName.c_str();
    out << "\033[37m"; // Gray for prompt symbol
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
    // Set text to bold directly with ANSI escape code
    // This is a more direct approach than using rang
    cout << "\033[1m";  // ANSI escape code for bold
    
    for (;;) {
        KAI_TRY {
            for (;;) {
                WritePrompt(cout);
                // Don't need to set bold here anymore
                string text;
                getline(cin, text);

                // Process the input, keeping the bold setting
                cout << Process(text.c_str()).c_str();
                
                // Set foreground color to gray for stack output
                cout << "\033[37m";  // ANSI gray color
                executor->PrintStack(cout);
                // Reset to default color but keep bold
                cout << "\033[39m";  // Default foreground color

                if (_end) return _endCode;
            }
        }
        KAI_CATCH(Exception::Base, E) { 
            // Use direct ANSI red color for errors, while keeping bold
            cout << "\033[31m"; // Red color
            KAI_TRACE_ERROR_1(E); 
            // Reset to default color
            cout << "\033[39m";
        }
        KAI_CATCH(exception, E) { 
            cout << "\033[31m"; // Red color
            KAI_TRACE_ERROR_1(E.what());
            cout << "\033[39m";
        }
        KAI_CATCH_ALL() { 
            cout << "\033[31m"; // Red color
            KAI_TRACE_ERROR() << " something went wrong";
            cout << "\033[39m";
        }
    }
}

void Console::RegisterTypes() {
    // built-ins
    _reg->AddClass<const ClassBase *>(Label("Class"));  // TODO: add _methods
    _reg->AddClass<void>(Label("Void"));
    _reg->AddClass<bool>(Label("Bool"));
    _reg->AddClass<int>(Label("Int"));
    _reg->AddClass<float>(Label("Float"));
    _reg->AddClass<Vector3>(Label("Vector3"));
    _reg->AddClass<Vector4>(Label("Vector4"));

    // system types
    // ObjectSet::Register(*registry);
    String::Register(*_reg);
    Object::Register(*_reg);
    Handle::Register(*_reg);
    Stack::Register(*_reg);
    Continuation::Register(*_reg);
    Label::Register(*_reg);
    Operation::Register(*_reg);
    Compiler::Register(*_reg);
    Executor::Register(*_reg);
    Pathname::Register(*_reg);
    BasePointerBase::Register(*_reg);
    Pair::Register(*_reg);
    FunctionBase::Register(*_reg);
    BasePointer<MethodBase>::Register(*_reg);
    BasePointer<PropertyBase>::Register(*_reg);
    BinaryStream::Register(*_reg);
    StringStream::Register(*_reg);
    Array::Register(*_reg);
    List::Register(*_reg);
    Map::Register(*_reg, "Map");

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

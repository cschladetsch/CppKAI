#pragma once

#include <KAI/Console/ConsoleColor.h>
#include <KAI/Core/Tree.h>
#include <KAI/Executor/Compiler.h>
#include <KAI/Executor/Executor.h>
#include <KAI/Language.h>

#include <string>
#include <vector>

KAI_BEGIN

struct Coloriser;

class Console : public Reflected {
    Tree tree;
    Registry *reg_;
    Pointer<Executor> executor;
    Pointer<Compiler> compiler;
    std::shared_ptr<Memory::IAllocator> alloc;
    Language language;

    std::vector<std::string> commandHistory;

   public:
    Console();
    Console(std::shared_ptr<Memory::IAllocator>);
    ~Console();

    void SetLanguage(Language lang);
    void SetLanguage(int lang);
    Language GetLanguage() const;

    void WritePrompt(std::ostream &out) const;
    String GetPrompt() const;
    String Process(const String &);
    String ProcessShellCommand(const String &text);
    String ExpandShellCommands(const String &text);
    String ProcessZshCommand(const String &text);
    String ExpandHistoryReferences(const String &text);
    String ParseHistoryExpansion(const String &text);
    std::vector<std::string> SplitIntoWords(const std::string &text);
    String ApplyWordDesignators(const std::string &command,
                                const std::string &designators);
    String ApplyModifiers(const String &text, const std::string &modifiers);
    String ProcessQuickSubstitution(const String &text);
    String SearchHistoryAnywhere(const String &pattern);
    String ProcessHistoryModifier(const String &text, char modifier);
    String ProcessSubstitutionModifier(const String &text,
                                       const std::string &pattern);
    std::string ExtractFilePath(const std::string &text);
    std::string currentCommand;  // For !# support
    bool shellMode = false;      // Toggle for shell mode
    Registry &GetRegistry() const { return *reg_; }
    Tree &GetTree() { return tree; }
    Tree const &GetTree() const { return tree; }

    Object GetRoot() const { return tree.GetRoot(); }

    Pointer<Executor> GetExecutor() const { return executor; }
    Pointer<Compiler> GetCompiler() const { return compiler; }

    Pointer<Continuation> Compile(const char *, Structure);
    void Execute(const String &text, Structure st = Structure::Statement);
    bool ExecuteFile(const char *);
    void Execute(Pointer<Continuation> cont);

    String WriteStack() const;
    void ShowColoredStack() const;
    void ControlC();
    static void Register(Registry &);

    int Run();
    
    // Helper method to detect incomplete structures for multi-line input
    bool IsStructureIncomplete(const String &text) const;

   protected:
    void Create();
    void CreateTree();
    void RegisterTypes();
    void ExposeTypesToTree(Object types);

   private:
    bool end_ = false;
    int endCode_ = 0;
};

KAI_TYPE_TRAITS(Console, Number::Console, Properties::Reflected);

KAI_END

#pragma once

#include <KAI/Console/ConsoleColor.h>
#include <KAI/Core/Tree.h>
#include <KAI/Executor/Compiler.h>
#include <KAI/Executor/Executor.h>
#include <KAI/Language.h>
#include <KAI/Language/Common/TranslatorCommon.h>
#include <KAI/Network/RakNetAdapter.h>

#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

KAI_BEGIN

struct Coloriser;

enum class NetworkMessageType : unsigned char {
    CONSOLE_COMMAND = RakNet::ID_USER_PACKET_ENUM + 10,
    CONSOLE_RESULT = RakNet::ID_USER_PACKET_ENUM + 11,
    CONSOLE_BROADCAST = RakNet::ID_USER_PACKET_ENUM + 12,
    CONSOLE_LANGUAGE_SWITCH = RakNet::ID_USER_PACKET_ENUM + 13
};

struct NetworkConsoleMessage {
    std::string senderId;
    std::string command;
    std::string result;
    Language language;
    long timestamp;

    NetworkConsoleMessage() : language(Language::Pi), timestamp(0) {}
};

class Console : public Reflected {
    Tree tree;
    Registry *reg_;
    Pointer<Executor> executor;
    Pointer<Compiler> compiler;
    std::shared_ptr<Memory::IAllocator> alloc;
    Language language;
    std::shared_ptr<TranslatorCommon> translator;

    std::vector<std::string> commandHistory;
    std::string historyFile;
    static const size_t maxHistorySize = 1000;

    // Network members
    RakNet::RakPeerInterface* peer_;
    std::mutex peersMutex_;
    std::vector<RakNet::SystemAddress> connectedPeers_;
    std::thread messageThread_;
    bool networkingEnabled_;
    bool networkRunning_;
    int listenPort_;
    std::string consoleId_;
    std::vector<NetworkConsoleMessage> messageHistory_;
    std::function<void(const NetworkConsoleMessage&)> messageCallback_;

   public:
    Console();
    Console(std::shared_ptr<Memory::IAllocator>);
    ~Console();

    void SetLanguage(Language lang);
    void SetLanguage(int lang);
    Language GetLanguage() const;

    void SetTranslator(std::shared_ptr<TranslatorCommon> trans);
    std::shared_ptr<TranslatorCommon> GetTranslator() const {
        return translator;
    }

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
    void ClearScreen() const;
    String ReadLineWithDynamicColor();
    void ExecuteShellCommandWithColor(const std::string &command);
    static void Register(Registry &);

    // Help system
    void ShowHelp(const std::string &topic = "") const;
    void ShowBasicHelp() const;
    void ShowHistoryHelp() const;
    void ShowLanguageHelp(const std::string &lang) const;
    void ShowBuiltinCommands() const;
    bool ProcessBuiltinCommand(const std::string &command);

    // History management
    void LoadHistory();
    void SaveHistory() const;
    void AddToHistory(const std::string &command);

    // Network functionality
    bool StartNetworking(int listenPort = 14600);
    bool ConnectToPeer(const std::string& host, int port);
    void StopNetworking();
    bool SendCommandToPeer(const std::string& peerAddr, const std::string& command);
    void BroadcastCommand(const std::string& command);
    std::vector<std::string> GetConnectedPeers() const;
    std::vector<NetworkConsoleMessage> GetNetworkHistory() const;
    void SetNetworkMessageCallback(std::function<void(const NetworkConsoleMessage&)> callback);
    String ProcessNetworkCommand(const String& command);
    void ShowNetworkHelp() const;
    bool IsNetworkingEnabled() const { return networkingEnabled_; }

    int Run();

    // Helper method to detect incomplete structures for multi-line input
    bool IsStructureIncomplete(const String &text) const;

   protected:
    void Create();
    void CreateTree();
    void RegisterTypes();
    void ExposeTypesToTree(Object types);

    // Network protected methods
    void ProcessNetworkMessages();
    void HandleNetworkPacket(RakNet::Packet* packet);
    void HandleConsoleCommand(RakNet::Packet* packet);
    void HandleConsoleResult(RakNet::Packet* packet);
    void HandleConsoleBroadcast(RakNet::Packet* packet);
    void HandleLanguageSwitch(RakNet::Packet* packet);
    void SendResultToPeer(const RakNet::SystemAddress& peer, const std::string& command, 
                         const std::string& result);
    void AddPeer(const RakNet::SystemAddress& address);
    void RemovePeer(const RakNet::SystemAddress& address);
    void LogNetworkMessage(const NetworkConsoleMessage& message);
    std::string GenerateConsoleId();
    std::string AddressToString(const RakNet::SystemAddress& addr) const;
    RakNet::SystemAddress FindPeerByAddress(const std::string& addr) const;

   private:
    bool end_ = false;
    int endCode_ = 0;
};

KAI_TYPE_TRAITS(Console, Number::Console, Properties::Reflected);

KAI_END

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "KAI/Console/Console.h"
#include "KAI/Language/Pi/PiTranslator.h"
#include "KAI/Language/Rho/RhoTranslator.h"
#include "rang.hpp"
using namespace std;
using namespace kai;

#if 0
#include "KAI/Network/Peer.h"

struct Peer : kai::Peer
{
    bool Start()
    {
        //return _peer.Start(ReceivePacket);
        return false;
    }

    bool Connect(String host, int port)
    {
        return _peer.Connect(host.c_str(), port);
    }

    void Send(String text)
    {
        _peer.SendText(text.c_str());
    }

    //void ReceivePacket(RakNet::Pac)
    //{
    //}

    kai::Peer _peer;
};
#endif

std::string KaiVersionString() {
    stringstream str;
    str << KAI_VERSION_MAJOR << '.' << KAI_VERSION_MINOR << '.'
        << KAI_VERSION_PATCH;
    return str.str();
}

void SetupLanguageTranslators(Console& console) {
    // Get the registry and compiler
    auto& reg = console.GetRegistry();
    auto compiler = console.GetCompiler();

    // Create translators for each language as shared pointers
    auto piTranslator = std::make_shared<PiTranslator>(reg);
    auto rhoTranslator = std::make_shared<RhoTranslator>(reg);

    // Set up the translation function
    compiler->SetTranslateFunction(
        [=](const String& text, Structure st) -> Pointer<Continuation> {
            int lang = compiler->GetLanguage();
            int traceLevel = compiler->GetTraceLevel();

            switch (static_cast<Language>(lang)) {
                case Language::Pi: {
                    piTranslator->trace = traceLevel;
                    auto result = piTranslator->Translate(text.c_str(), st);
                    if (piTranslator->Failed) {
                        KAI_TRACE_ERROR() << piTranslator->Error;
                        return Object();
                    }
                    return result;
                }
                case Language::Rho: {
                    rhoTranslator->trace = traceLevel;
                    auto result = rhoTranslator->Translate(text.c_str(), st);
                    if (rhoTranslator->Failed) {
                        KAI_TRACE_ERROR() << rhoTranslator->Error;
                        return Object();
                    }
                    return result;
                }
                default:
                    return Object();
            }
        });
}

int main(int argc, char** argv) {
    // Create console here instead of as global
    Console console;

    // the higher the number, the greater the verbosity of debug output for
    // language systems
    Process::trace = 0;

    // the higher the number, the greater the verbosity of debug output for the
    // Executor
    console.GetExecutor()->SetTraceLevel(0);

    // Set up language translators
    SetupLanguageTranslators(console);

    // Check if a file argument was provided
    if (argc > 1) {
        // Execute file as a program
        std::string filename = argv[1];

        // Determine language from file extension
        if (filename.ends_with(".pi")) {
            console.SetLanguage(Language::Pi);
        } else if (filename.ends_with(".rho")) {
            console.SetLanguage(Language::Rho);
        } else {
            std::cerr << "Unknown file extension. Expected .pi or .rho\n";
            return 1;
        }

        // Use ExecuteFile which now supports shell commands
        if (!console.ExecuteFile(filename.c_str())) {
            std::cerr << "Failed to execute file: " << filename << "\n";
            return 1;
        }
        return 0;
    }

    // No file argument - start REPL with Pi as default
    console.SetLanguage(Language::Pi);

    // start the REPL
    return console.Run();
}

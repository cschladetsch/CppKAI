#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "KAI/Console/Console.h"
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

Console console;

std::string KaiVersionString() {
    stringstream str;
    str << KAI_VERSION_MAJOR << '.' << KAI_VERSION_MINOR << '.'
        << KAI_VERSION_PATCH;
    return str.str();
}

int main(int argc, char **argv) {
    // Apply bold style from the start
    cout << rang::style::bold << "KAI v" << KaiVersionString() << "\n\n";

    // the higher the number, the greater the verbosity of debug output for
    // language systems
    Process::trace = 0;

    // the higher the number, the greater the verbosity of debug output for the
    // Executor
    console.GetExecutor()->SetTraceLevel(0);

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

        // Read the file
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Could not open file: " << filename << "\n";
            return 1;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string program = buffer.str();

        // Execute as a complete program (not individual statements)
        console.Execute(program, Structure::Program);
        return 0;
    }

    // No file argument - start REPL with Pi as default
    console.SetLanguage(Language::Pi);

    // start the REPL
    return console.Run();
}

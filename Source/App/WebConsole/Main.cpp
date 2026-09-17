/// WebConsole - TCP shim exposing a KAI Console over a raw text socket
///
/// Protocol (newline-delimited, UTF-8):
///   Browser -> bridge -> WebConsole:  <expression>\n
///   WebConsole -> bridge -> Browser:  RESULT <output>\n  or  ERROR <msg>\n
///
/// Usage: WebConsole [--port PORT] [--lang pi|rho] [--trace N]

#include <iostream>
#include <string>
#include <thread>

#ifdef _WIN32
#  include <winsock2.h>
#  include <ws2tcpip.h>
#  pragma comment(lib, "ws2_32.lib")
   using socklen_t = int;
#else
#  include <arpa/inet.h>
#  include <netinet/in.h>
#  include <sys/socket.h>
#  include <unistd.h>
   using SOCKET = int;
   static constexpr SOCKET INVALID_SOCKET = -1;
   static constexpr int    SOCKET_ERROR   = -1;
   inline void closesocket(SOCKET s) { ::close(s); }
#endif

#include "KAI/Console/Console.h"
#include "KAI/Executor/Continuation.h"
#include "KAI/Language/Common/TranslatorFactory.h"
#include "KAI/Language/Pi/PiTranslator.h"
#include "KAI/Language/Rho/RhoTranslator.h"

using namespace std;
using namespace kai;

REGISTER_TRANSLATOR(Language::Pi,  PiTranslator)
REGISTER_TRANSLATOR(Language::Rho, RhoTranslator)

static void SendLine(SOCKET sock, const string& line) {
    string out = line + "\n";
    ::send(sock, out.data(), static_cast<int>(out.size()), 0);
}

static string StackTopAsString(Console& console) {
    auto* exec = &*console.GetExecutor();
    int stackSize = exec->GetDataStack()->Size();
    if (stackSize == 0) return "ok";

    StringStream str;
    for (int i = 0; i < stackSize; i++) {
        int displayIndex = stackSize - 1 - i;
        auto obj = exec->GetDataStack()->At(displayIndex);
        str << "[" << displayIndex << "]: " << obj << "\n";
    }
    String kai_s = str.ToString();
    string s = kai_s.c_str();
    while (!s.empty() && (s.back() == '\n' || s.back() == '\r')) s.pop_back();
    return s;
}

static string Evaluate(Console& console, Language& currentLang, const string& raw) {
    string src = raw;
    Language lang = currentLang;

    if (src.starts_with("%rho ") || src == "%rho") { lang = Language::Rho; src = src.substr(5); }
    else if (src.starts_with("%pi ") || src == "%pi") { lang = Language::Pi; src = src.substr(4); }

    if (src.empty()) return "RESULT ok";

    if (lang != console.GetLanguage()) {
        console.SetLanguage(lang);
        auto t = TranslatorFactory::Instance().CreateTranslator(lang, console.GetRegistry());
        if (t) console.SetTranslator(t);
        currentLang = lang;
    }

    try {
        auto cont = console.Compile(src.c_str(), Structure::Program);
        if (!cont.Exists())
            return "ERROR compile failed";

        console.GetExecutor()->Continue(Value<Continuation>(cont));

        return "RESULT " + StackTopAsString(console);
    } catch (const exception& e) {
        return string("ERROR ") + e.what();
    } catch (...) {
        return "ERROR unknown exception";
    }
}

static void HandleClient(SOCKET client, int traceLevel, Language defaultLang) {
    Console console;
    Process::trace = 0;
    console.GetExecutor()->SetTraceLevel(traceLevel);
    console.SetLanguage(defaultLang);

    auto translator = TranslatorFactory::Instance().CreateTranslator(defaultLang, console.GetRegistry());
    if (translator) console.SetTranslator(translator);

    Language currentLang = defaultLang;

    SendLine(client, "READY kai-webconsole");

    string linebuf;
    char chunk[4096];

    while (true) {
        int n = ::recv(client, chunk, sizeof(chunk) - 1, 0);
        if (n <= 0) break;

        chunk[n] = '\0';
        linebuf += chunk;

        size_t pos;
        while ((pos = linebuf.find('\n')) != string::npos) {
            string line = linebuf.substr(0, pos);
            linebuf.erase(0, pos + 1);
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty()) continue;

            string response = Evaluate(console, currentLang, line);
            SendLine(client, response);
        }
    }

    closesocket(client);
}

int main(int argc, char** argv) {
#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
#endif

    int      port        = 7272;
    int      traceLevel  = 0;
    Language defaultLang = Language::Pi;

    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        auto next = [&]() -> string { return (i + 1 < argc) ? argv[++i] : ""; };
        if      (arg == "--port")  port       = stoi(next());
        else if (arg == "--trace") traceLevel = stoi(next());
        else if (arg == "--lang")  { if (next() == "rho") defaultLang = Language::Rho; }
    }

    SOCKET server = ::socket(AF_INET, SOCK_STREAM, 0);
    if (server == INVALID_SOCKET) { cerr << "socket() failed\n"; return 1; }

    int yes = 1;
    ::setsockopt(server, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&yes), sizeof(yes));

    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(static_cast<uint16_t>(port));
    addr.sin_addr.s_addr = INADDR_ANY;

    if (::bind(server, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR) {
        cerr << "bind() failed on port " << port << "\n"; return 1;
    }
    if (::listen(server, 8) == SOCKET_ERROR) {
        cerr << "listen() failed\n"; return 1;
    }

    cout << "kai-webconsole listening on port " << port
         << " (lang=" << (defaultLang == Language::Pi ? "pi" : "rho")
         << ", trace=" << traceLevel << ")\n";
    cout.flush();

    while (true) {
        sockaddr_in peer{};
        socklen_t peerLen = sizeof(peer);
        SOCKET client = ::accept(server, reinterpret_cast<sockaddr*>(&peer), &peerLen);
        if (client == INVALID_SOCKET) continue;

        char peerIp[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &peer.sin_addr, peerIp, sizeof(peerIp));
        cout << "client connected: " << peerIp << ":" << ntohs(peer.sin_port) << "\n";
        cout.flush();

        thread(HandleClient, client, traceLevel, defaultLang).detach();
    }

#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}

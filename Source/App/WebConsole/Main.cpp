/// WebConsole - TCP shim exposing a KAI Console over a raw text socket
///
/// Sits between kai-bridge (Rust/WebSocket) and a local KAI Registry+Executor.
/// Protocol (newline-delimited, UTF-8):
///
///   Browser → bridge → WebConsole:
///     <expression>\n
///     e.g.  1 2 +\n
///     e.g.  %rho x := 42\n      (prefix %rho to switch to Rho for one line)
///     e.g.  %pi  1 2 +\n        (prefix %pi  to switch to Pi  for one line)
///
///   WebConsole → bridge → Browser:
///     RESULT <output>\n          on success (output is the top-of-stack or "ok")
///     ERROR  <message>\n         on parse/eval failure
///     SUP <addr> <json>\n        unsolicited, when object state changes (future)
///
/// Usage:
///   WebConsole [--port PORT] [--lang pi|rho] [--trace N]
///   Defaults: port=7272, lang=pi, trace=0
///
/// Build: add this file and CMakeLists.txt.in to CppKAI on the webui branch.

#include <atomic>
#include <chrono>
#include <iostream>
#include <sstream>
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
#include "KAI/Core/Object.h"
#include "KAI/Executor/Continuation.h"
#include "KAI/Language/Common/TranslatorFactory.h"
#include "KAI/Language/Pi/PiTranslator.h"
#include "KAI/Language/Rho/RhoTranslator.h"

using namespace std;
using namespace kai;

REGISTER_TRANSLATOR(Language::Pi,  PiTranslator)
REGISTER_TRANSLATOR(Language::Rho, RhoTranslator)

// ── Helpers ──────────────────────────────────────────────────────────────────

static void SendLine(SOCKET sock, const string& line) {
    string out = line + "\n";
    ::send(sock, out.data(), static_cast<int>(out.size()), 0);
}

/// Capture whatever the executor leaves on top of the data stack as a string.
static string StackTopAsString(Console& console) {
    auto stack = console.GetExecutor()->GetDataStack();
    if (!stack.Exists() || stack->Empty())
        return "ok";

    // Redirect cout so any Print() calls inside KAI go to our buffer
    ostringstream buf;
    streambuf* old = cout.rdbuf(buf.rdbuf());

    try {
        // Use KAI's own StringStream to convert the top-of-stack object
        Object top = stack->Top();
        StringStream kai_str;
        kai_str << top;
        cout.rdbuf(old);
        String kai_s = kai_str.ToString();
        string s = kai_s.c_str();
        if (s.empty()) s = buf.str();
        if (s.empty()) s = "ok";
        return s;
    } catch (...) {
        cout.rdbuf(old);
        return buf.str().empty() ? "ok" : buf.str();
    }
}

// ── Evaluate one expression, return RESULT/ERROR line ────────────────────────

static string Evaluate(Console& console, Language& currentLang,
                        const string& raw) {
    string src = raw;

    // Per-line language override: %pi / %rho prefix
    Language lang = currentLang;
    if (src.starts_with("%rho ") || src == "%rho") {
        lang = Language::Rho;
        src  = src.substr(5);
    } else if (src.starts_with("%pi ") || src == "%pi") {
        lang = Language::Pi;
        src  = src.substr(4);
    }

    if (src.empty())
        return "RESULT ok";

    // Set language if it changed
    if (lang != console.GetLanguage()) {
        console.SetLanguage(lang);
        auto t = TranslatorFactory::Instance().CreateTranslator(lang,
                                                console.GetRegistry());
        if (t) console.SetTranslator(t);
    }

    try {
        auto cont = console.Compile(src.c_str(), Structure::Expression);
        if (!cont.Exists())
            return "ERROR compile failed";

        console.GetExecutor()->ClearStacks();
        console.GetExecutor()->Continue(Value<Continuation>(cont));

        return "RESULT " + StackTopAsString(console);

    } catch (const exception& e) {
        return string("ERROR ") + e.what();
    } catch (...) {
        return "ERROR unknown exception";
    }
}

// ── Per-connection handler (runs in its own thread) ───────────────────────────

static void HandleClient(SOCKET client, int traceLevel, Language defaultLang) {
    // Each connection gets its own Console (own Registry + Executor).
    // This keeps connections isolated; state does not persist between sessions.
    // If you want a shared Registry, move Console to the caller and add a mutex.
    Console console;
    Process::trace = 0;
    console.GetExecutor()->SetTraceLevel(traceLevel);
    console.SetLanguage(defaultLang);

    auto translator = TranslatorFactory::Instance().CreateTranslator(
                          defaultLang, console.GetRegistry());
    if (translator)
        console.SetTranslator(translator);

    Language currentLang = defaultLang;

    // Send a greeting so the bridge knows we're alive
    SendLine(client, "READY kai-webconsole");

    string linebuf;
    char   chunk[4096];

    while (true) {
        int n = ::recv(client, chunk, sizeof(chunk) - 1, 0);
        if (n <= 0) break;   // connection closed or error

        chunk[n] = '\0';
        linebuf += chunk;

        // Process every complete line
        size_t pos;
        while ((pos = linebuf.find('\n')) != string::npos) {
            string line = linebuf.substr(0, pos);
            linebuf.erase(0, pos + 1);

            // Strip \r if present (Windows line endings from the bridge)
            if (!line.empty() && line.back() == '\r')
                line.pop_back();

            if (line.empty()) continue;

            string response = Evaluate(console, currentLang, line);
            SendLine(client, response);
        }
    }

    closesocket(client);
}

// ── main ─────────────────────────────────────────────────────────────────────

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
        auto next = [&]() -> string {
            return (i + 1 < argc) ? argv[++i] : "";
        };
        if (arg == "--port")  port       = stoi(next());
        else if (arg == "--trace") traceLevel = stoi(next());
        else if (arg == "--lang") {
            string l = next();
            if (l == "rho") defaultLang = Language::Rho;
        }
    }

    SOCKET server = ::socket(AF_INET, SOCK_STREAM, 0);
    if (server == INVALID_SOCKET) {
        cerr << "socket() failed\n"; return 1;
    }

    int yes = 1;
    ::setsockopt(server, SOL_SOCKET, SO_REUSEADDR,
                 reinterpret_cast<const char*>(&yes), sizeof(yes));

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
        socklen_t   peerLen = sizeof(peer);
        SOCKET client = ::accept(server,
                                 reinterpret_cast<sockaddr*>(&peer), &peerLen);
        if (client == INVALID_SOCKET) continue;

        char peerIp[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &peer.sin_addr, peerIp, sizeof(peerIp));
        cout << "client connected: " << peerIp
             << ":" << ntohs(peer.sin_port) << "\n";
        cout.flush();

        // Detach thread - no join needed, connection is self-contained
        thread(HandleClient, client, traceLevel, defaultLang).detach();
    }

#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}

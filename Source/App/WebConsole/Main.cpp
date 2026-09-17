/// WebConsole - TCP shim exposing a KAI Console over a binary-framed socket
///
/// Protocol:
///   Greeting (plain text, one line):  READY kai-webconsole\n
///   Inbound  (plain text):            <expression>\n
///   Outbound (binary frames):         [4-byte LE length][payload]
///
///   Payload kinds:
///     0x01  Result  - 1-byte kind | src string | top-of-stack Object
///     0x02  Error   - 1-byte kind | src string | error message string
///     0x03  Stack   - 1-byte kind | src string | full stack as KAI Array
///
/// Usage: WebConsole [--port PORT] [--lang pi|rho] [--trace N]

#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <cstdint>

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

#include "KAI/Core/BinaryStream.h"
#include "KAI/Console/Console.h"
#include "KAI/Executor/Continuation.h"
#include "KAI/Language/Common/TranslatorFactory.h"
#include "KAI/Language/Pi/PiTranslator.h"
#include "KAI/Language/Rho/RhoTranslator.h"

using namespace std;
using namespace kai;

REGISTER_TRANSLATOR(Language::Pi,  PiTranslator)
REGISTER_TRANSLATOR(Language::Rho, RhoTranslator)

// ── helpers ───────────────────────────────────────────────────────────────────

static void SendRaw(SOCKET sock, const void* data, int len) {
    ::send(sock, reinterpret_cast<const char*>(data), len, 0);
}

static void SendLine(SOCKET sock, const string& line) {
    string out = line + "\n";
    SendRaw(sock, out.data(), static_cast<int>(out.size()));
}

/// Write a length-prefixed UTF-8 string into a BinaryStream.
static void WriteString(BinaryStream& bs, const string& s) {
    uint32_t len = static_cast<uint32_t>(s.size());
    bs.Write(len);
    if (len > 0) bs.Write(static_cast<int>(len), s.data());
}

/// Send a binary frame: 4-byte LE length + payload bytes.
static void SendFrame(SOCKET sock, BinaryStream& bs) {
    // BinaryStream inherits Begin()/Last() from BinaryPacket
    const char* begin = bs.Begin();
    const char* last  = bs.Last();
    if (!begin || last <= begin) return;

    uint32_t payloadLen = static_cast<uint32_t>(last - begin);
    // Write length LE
    uint8_t header[4];
    header[0] = payloadLen & 0xff;
    header[1] = (payloadLen >> 8) & 0xff;
    header[2] = (payloadLen >> 16) & 0xff;
    header[3] = (payloadLen >> 24) & 0xff;
    SendRaw(sock, header, 4);
    SendRaw(sock, begin, static_cast<int>(payloadLen));
}

// ── frame builders ────────────────────────────────────────────────────────────

static void SendResultFrame(SOCKET sock, Console& console,
                            const string& src, Object top) {
    BinaryStream bs(console.GetRegistry());
    uint8_t kind = 0x01;
    bs.Write(kind);
    WriteString(bs, src);
    bs << top;
    SendFrame(sock, bs);
}

static void SendErrorFrame(SOCKET sock, Console& console,
                           const string& src, const string& msg) {
    BinaryStream bs(console.GetRegistry());
    uint8_t kind = 0x02;
    bs.Write(kind);
    WriteString(bs, src);
    WriteString(bs, msg);
    SendFrame(sock, bs);
}

static void SendStackFrame(SOCKET sock, Console& console, const string& src) {
    auto* exec = &*console.GetExecutor();
    int stackSize = exec->GetDataStack()->Size();

    // Build a KAI Array of stack items (top first)
    Value<Array> arr = console.GetRegistry().New<Array>();
    for (int i = stackSize - 1; i >= 0; --i) {
        arr->Append(exec->GetDataStack()->At(i));
    }

    BinaryStream bs(console.GetRegistry());
    uint8_t kind = 0x03;
    bs.Write(kind);
    WriteString(bs, src);
    bs << *arr;
    SendFrame(sock, bs);
}

// ── evaluate ──────────────────────────────────────────────────────────────────

static void Evaluate(SOCKET sock, Console& console, Language& currentLang,
                     const string& raw) {
    string src = raw;
    Language lang = currentLang;

    if (src.size() >= 5 && src.substr(0,5) == "%rho ") { lang = Language::Rho; src = src.substr(5); }
    else if (src == "%rho") { lang = Language::Rho; src = ""; }
    else if (src.size() >= 4 && src.substr(0,4) == "%pi ") { lang = Language::Pi; src = src.substr(4); }
    else if (src == "%pi") { lang = Language::Pi; src = ""; }

    if (src.empty()) {
        SendStackFrame(sock, console, src);
        return;
    }

    if (lang != console.GetLanguage()) {
        console.SetLanguage(lang);
        auto t = TranslatorFactory::Instance().CreateTranslator(lang, console.GetRegistry());
        if (t) console.SetTranslator(t);
        currentLang = lang;
    }

    try {
        auto cont = console.Compile(src.c_str(), Structure::Program);
        if (!cont.Exists()) {
            SendErrorFrame(sock, console, src, "compile failed");
            return;
        }

        console.GetExecutor()->Continue(Value<Continuation>(cont));

        // Send top-of-stack as result, plus full stack frame
        auto* exec = &*console.GetExecutor();
        int stackSize = exec->GetDataStack()->Size();
        if (stackSize > 0) {
            Object top = exec->GetDataStack()->At(stackSize - 1);
            SendResultFrame(sock, console, src, top);
        } else {
            SendErrorFrame(sock, console, src, "ok");
        }
        SendStackFrame(sock, console, src);

    } catch (const exception& e) {
        SendErrorFrame(sock, console, src, e.what());
    } catch (...) {
        SendErrorFrame(sock, console, src, "unknown exception");
    }
}

// ── per-connection handler ────────────────────────────────────────────────────

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
            Evaluate(client, console, currentLang, line);
        }
    }

    closesocket(client);
}

// ── main ──────────────────────────────────────────────────────────────────────

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

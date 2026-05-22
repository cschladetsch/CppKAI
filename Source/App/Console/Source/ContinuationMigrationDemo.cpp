#include <atomic>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>

#include "KAI/Console/Console.h"
#include "KAI/Executor/BinBase.h"
#include "KAI/Executor/Continuation.h"
#include "KAI/Network/ConnectionEvent.h"
#include "KAI/Network/Node.h"

using namespace kai;
using namespace kai::net;

namespace {
struct Options {
    enum class Mode { Server, Client };

    Mode mode = Mode::Server;
    std::string host = "127.0.0.1";
    int port = 17100;
    int handle = 1;
    int timeout_ms = 15000;
};

void PrintUsage(const char *program) {
    std::cerr << "Usage: " << program
              << " --server|--client [--host HOST] [--port PORT] [--handle N]"
              << " [--timeout MS]\n";
}

Options ParseOptions(int argc, char **argv) {
    Options options;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        auto requireValue = [&](const char *name) -> std::string {
            if (i + 1 >= argc) {
                throw std::runtime_error(std::string("Missing value for ") +
                                         name);
            }
            return argv[++i];
        };

        if (arg == "--server") {
            options.mode = Options::Mode::Server;
        } else if (arg == "--client") {
            options.mode = Options::Mode::Client;
        } else if (arg == "--host") {
            options.host = requireValue("--host");
        } else if (arg == "--port") {
            options.port = std::stoi(requireValue("--port"));
        } else if (arg == "--handle") {
            options.handle = std::stoi(requireValue("--handle"));
        } else if (arg == "--timeout") {
            options.timeout_ms = std::stoi(requireValue("--timeout"));
        } else if (arg == "--help" || arg == "-h") {
            PrintUsage(argv[0]);
            std::exit(0);
        } else {
            throw std::runtime_error("Unknown argument: " + arg);
        }
    }
    return options;
}

bool WaitForConnection(Node &node, bool &connected, int timeout_ms) {
    using clock = std::chrono::steady_clock;
    const auto start = clock::now();
    while (!connected) {
        node.Update();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        if (clock::now() - start > std::chrono::milliseconds(timeout_ms)) {
            return false;
        }
    }
    return true;
}

int RunServer(const Options &options) {
    Console console;
    console.SetLanguage(Language::Pi);

    Node node;
    node.SetRegistry(&console.GetRegistry());
    node.Listen(IpAddress(options.host), options.port);
    if (!node.IsRunning()) {
        std::cerr << "SERVER_ERROR: failed to listen on " << options.host << ":"
                  << options.port << "\n";
        return 1;
    }

    const NetHandle agentHandle = node.AttachAgent(nullptr);
    std::atomic<bool> completed{false};
    std::atomic<int> migratedResult{0};

    node.RegisterMethod<int, Object>(
        agentHandle, "ThawAndResume",
        std::function<int(Object)>([&](Object frozen) {
            try {
                Object cont = Bin::Thaw(frozen);
                if (!cont.Exists() || !cont.IsType<Continuation>()) {
                    throw std::runtime_error("Expected a thawed continuation");
                }

                console.GetExecutor()->ClearStacks();
                console.GetExecutor()->Continue(Value<Continuation>(cont));

                auto stack = console.GetExecutor()->GetDataStack();
                if (!stack.Exists() || stack->Empty()) {
                    throw std::runtime_error("Continuation produced no result");
                }

                int result = ConstDeref<int>(stack->Top());
                migratedResult = result;
                completed = true;
                std::cout << "SERVER_RESULT=" << result << "\n";
                return result;
            } catch (const std::exception &e) {
                std::cerr << "SERVER_ERROR: " << e.what() << "\n";
                throw;
            }
        }));

    std::cout << "SERVER_READY port=" << options.port
              << " handle=" << agentHandle.value << "\n";

    using clock = std::chrono::steady_clock;
    const auto start = clock::now();
    while (!completed) {
        node.Update();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        if (clock::now() - start >
            std::chrono::milliseconds(options.timeout_ms)) {
            std::cerr << "SERVER_ERROR: timed out waiting for migration\n";
            return 1;
        }
    }

    std::cout << "SERVER_DONE result=" << migratedResult.load() << "\n";
    return 0;
}

int RunClient(const Options &options) {
    Console console;
    console.SetLanguage(Language::Pi);

    Node node;
    node.SetRegistry(&console.GetRegistry());

    bool connected = false;
    node.SetConnectionEventCallback(
        [&](ConnectionEvent ev, const NetAddress &) {
            if (ev == ConnectionEvent::Connected) {
                connected = true;
            }
        });

    node.Connect(IpAddress(options.host), options.port);
    if (!WaitForConnection(node, connected, options.timeout_ms)) {
        std::cerr << "CLIENT_ERROR: timed out connecting to " << options.host
                  << ":" << options.port << "\n";
        return 1;
    }

    NetHandle agentHandle(options.handle);
    node.BindProxyAddress(agentHandle, NetAddress(options.host, options.port));

    const char *expression = "{ 2 * } 'double # 5 double &";
    auto cont = console.Compile(expression, Structure::Program);
    if (!cont.Exists()) {
        std::cerr << "CLIENT_ERROR: failed to compile continuation\n";
        return 1;
    }

    Object frozen = Bin::Freeze(*cont->Self);
    if (!frozen.Exists()) {
        std::cerr << "CLIENT_ERROR: failed to freeze continuation\n";
        return 1;
    }

    auto future = node.Invoke<int>(agentHandle, "ThawAndResume", frozen);
    using clock = std::chrono::steady_clock;
    const auto start = clock::now();
    while (!future.IsComplete()) {
        node.Update();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        if (clock::now() - start >
            std::chrono::milliseconds(options.timeout_ms)) {
            std::cerr << "CLIENT_ERROR: timed out waiting for response\n";
            return 1;
        }
    }

    if (!future.Succeeded()) {
        std::cerr << "CLIENT_ERROR: remote call failed";
        if (!future.GetErrorMessage().empty()) {
            std::cerr << ": " << future.GetErrorMessage();
        }
        std::cerr << "\n";
        return 1;
    }

    int result = future.GetValue();
    std::cout << "CLIENT_RESULT=" << result << "\n";
    if (result != 10) {
        std::cerr << "CLIENT_ERROR: expected 10, got " << result << "\n";
        return 1;
    }

    std::cout << "MIGRATION_OK result=" << result << "\n";
    return 0;
}
}  // namespace

int main(int argc, char **argv) {
    try {
        std::cout << std::unitbuf;
        std::cerr << std::unitbuf;
        Options options = ParseOptions(argc, argv);
        switch (options.mode) {
            case Options::Mode::Server:
                return RunServer(options);
            case Options::Mode::Client:
                return RunClient(options);
        }
    } catch (const std::exception &e) {
        std::cerr << "ERROR: " << e.what() << "\n";
        PrintUsage(argv[0]);
        return 1;
    }

    return 1;
}

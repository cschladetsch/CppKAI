#include <KAI/Network/Network.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

KAI_NET_BEGIN

// Simple proxy code generator that works with Tau interface files
class ProxyGenerator {
   private:
    struct Method {
        std::string returnType;
        std::string name;
        std::vector<std::pair<std::string, std::string>> params;  // type, name
    };

    struct Class {
        std::string name;
        std::vector<Method> methods;
    };

    struct Namespace {
        std::string name;
        std::vector<Class> classes;
    };

    std::vector<Namespace> namespaces;

    // Extract namespaces, classes, and methods from Tau file
    bool ParseTauFile(const std::string& content) {
        try {
            std::istringstream stream(content);
            std::string line;

            Namespace* currentNamespace = nullptr;
            Class* currentClass = nullptr;

            // Simple regex patterns to match Tau syntax
            std::regex namespacePattern(R"(\s*namespace\s+(\w+)\s*\{)");
            std::regex classPattern(R"(\s*class\s+(\w+)\s*\{)");
            std::regex methodPattern(
                R"(\s*(\w+(?:<[\w\s,<>]+>)?)\s+(\w+)\s*\(([^)]*)\)\s*;)");
            std::regex paramPattern(R"((\w+(?:<[\w\s,<>]+>)?)\s+(\w+))");
            std::regex closeBracePattern(R"(\s*\}\s*)");

            while (std::getline(stream, line)) {
                std::smatch match;

                // Check for namespace
                if (std::regex_search(line, match, namespacePattern)) {
                    namespaces.push_back({match[1]});
                    currentNamespace = &namespaces.back();
                    currentClass = nullptr;
                }
                // Check for class
                else if (currentNamespace &&
                         std::regex_search(line, match, classPattern)) {
                    currentNamespace->classes.push_back({match[1]});
                    currentClass = &currentNamespace->classes.back();
                }
                // Check for method
                else if (currentClass &&
                         std::regex_search(line, match, methodPattern)) {
                    Method method;
                    method.returnType = match[1];
                    method.name = match[2];

                    // Parse parameters
                    std::string params = match[3];
                    if (!params.empty()) {
                        std::istringstream paramStream(params);
                        std::string param;

                        while (std::getline(paramStream, param, ',')) {
                            std::smatch paramMatch;
                            if (std::regex_search(param, paramMatch,
                                                  paramPattern)) {
                                method.params.push_back(
                                    {paramMatch[1], paramMatch[2]});
                            }
                        }
                    }

                    currentClass->methods.push_back(method);
                }
                // Check for closing braces
                else if (std::regex_search(line, match, closeBracePattern)) {
                    if (currentClass) {
                        currentClass = nullptr;
                    } else if (currentNamespace) {
                        currentNamespace = nullptr;
                    }
                }
            }

            return !namespaces.empty();
        } catch (const std::exception& e) {
            std::cerr << "Error parsing Tau file: " << e.what() << std::endl;
            return false;
        }
    }

    // Generate proxy class for a given class
    std::string GenerateProxyClass(const Namespace& ns, const Class& cls) {
        std::stringstream ss;

        // Generate class header
        ss << "// Generated proxy for " << ns.name << "::" << cls.name << "\n";
        ss << "class " << cls.name << "Proxy {\n";
        ss << "private:\n";
        ss << "    kai::net::NetHandle _handle;\n";
        ss << "    kai::net::Node* _node;\n\n";

        ss << "public:\n";
        ss << "    " << cls.name
           << "Proxy(kai::net::Node* node, kai::net::NetHandle handle)\n";
        ss << "        : _node(node), _handle(handle) {}\n\n";

        // Generate proxy methods
        for (const auto& method : cls.methods) {
            // Method signature
            ss << "    " << method.returnType << " " << method.name << "(";

            // Method parameters
            for (size_t i = 0; i < method.params.size(); ++i) {
                const auto& param = method.params[i];
                ss << "const " << param.first << "& " << param.second;
                if (i < method.params.size() - 1) {
                    ss << ", ";
                }
            }
            ss << ") {\n";

            // Method implementation
            ss << "        // Create parameter packet\n";
            ss << "        RakNet::BitStream bs;\n";
            ss << "        bs.Write((unsigned "
                  "char)kai::net::NetworkSerializer::ID_KAI_FUNCTION_CALL);\n";
            ss << "        bs.Write(_handle);\n";
            ss << "        bs.Write(std::string(\"" << method.name << "\"));\n";

            // Serialize parameters
            for (const auto& param : method.params) {
                ss << "        bs.Write(" << param.second << ");\n";
            }

            // Send request and handle response
            if (method.returnType == "void") {
                ss << "        _node->Send(_handle, bs);\n";
            } else {
                ss << "        // Send and wait for response\n";
                ss << "        auto future = _node->SendWithResponse<"
                   << method.returnType << ">(_handle, bs);\n";
                ss << "        return future.get();\n";
            }

            ss << "    }\n\n";
        }

        ss << "};\n";

        return ss.str();
    }

    // Generate agent class for a given class
    std::string GenerateAgentClass(const Namespace& ns, const Class& cls) {
        std::stringstream ss;

        // Generate class header
        ss << "// Generated agent for " << ns.name << "::" << cls.name << "\n";
        ss << "class " << cls.name << "Agent {\n";
        ss << "private:\n";
        ss << "    " << ns.name << "::" << cls.name << "* _implementation;\n";
        ss << "    kai::net::Node* _node;\n\n";

        ss << "public:\n";
        ss << "    " << cls.name << "Agent(kai::net::Node* node, " << ns.name
           << "::" << cls.name << "* impl)\n";
        ss << "        : _node(node), _implementation(impl) {}\n\n";

        ss << "    void ProcessMessage(RakNet::BitStream& bs, "
              "RakNet::SystemAddress& sender) {\n";
        ss << "        // Read function name\n";
        ss << "        std::string functionName;\n";
        ss << "        bs.Read(functionName);\n\n";

        ss << "        // Dispatch to appropriate handler\n";
        ss << "        if (false) {}\n";

        // Generate function dispatch
        for (const auto& method : cls.methods) {
            ss << "        else if (functionName == \"" << method.name
               << "\") {\n";
            ss << "            Handle_" << method.name << "(bs, sender);\n";
            ss << "        }\n";
        }

        ss << "    }\n\n";

        // Generate handler methods
        for (const auto& method : cls.methods) {
            ss << "    void Handle_" << method.name
               << "(RakNet::BitStream& bs, RakNet::SystemAddress& sender) {\n";

            // Deserialize parameters
            for (const auto& param : method.params) {
                ss << "        " << param.first << " " << param.second << ";\n";
                ss << "        bs.Read(" << param.second << ");\n";
            }

            // Call implementation
            if (method.returnType == "void") {
                ss << "        _implementation->" << method.name << "(";
            } else {
                ss << "        " << method.returnType
                   << " result = _implementation->" << method.name << "(";
            }

            // Function parameters
            for (size_t i = 0; i < method.params.size(); ++i) {
                ss << method.params[i].second;
                if (i < method.params.size() - 1) {
                    ss << ", ";
                }
            }
            ss << ");\n";

            // Send response if needed
            if (method.returnType != "void") {
                ss << "        // Send response\n";
                ss << "        RakNet::BitStream response;\n";
                ss << "        response.Write((unsigned "
                      "char)kai::net::NetworkSerializer::ID_KAI_FUNCTION_"
                      "RESPONSE);\n";
                ss << "        response.Write(result);\n";
                ss << "        _node->Send(sender, response);\n";
            }

            ss << "    }\n\n";
        }

        ss << "};\n";

        return ss.str();
    }

   public:
    bool ProcessFile(const std::string& inputFile,
                     const std::string& outputFile) {
        try {
            // Read input file
            std::ifstream in(inputFile);
            if (!in) {
                std::cerr << "Failed to open input file: " << inputFile
                          << std::endl;
                return false;
            }

            std::string content((std::istreambuf_iterator<char>(in)),
                                std::istreambuf_iterator<char>());
            in.close();

            // Parse Tau file
            if (!ParseTauFile(content)) {
                std::cerr << "Failed to parse Tau file: " << inputFile
                          << std::endl;
                return false;
            }

            // Generate proxy and agent classes
            std::stringstream ss;
            ss << "// Generated network proxy code from " << inputFile << "\n";
            ss << "// DO NOT EDIT - This file was automatically generated\n\n";

            ss << "#pragma once\n\n";
            ss << "#include <KAI/Network/Network.h>\n";
            ss << "#include <KAI/Network/Serialization.h>\n";
            ss << "#include <string>\n\n";

            // Include original interface definitions
            for (const auto& ns : namespaces) {
                ss << "namespace " << ns.name << " {\n\n";

                // Forward declare classes
                for (const auto& cls : ns.classes) {
                    ss << "class " << cls.name << ";\n";
                }

                ss << "\n} // namespace " << ns.name << "\n\n";
            }

            // Generate proxy classes
            ss << "namespace kai {\n";
            ss << "namespace net {\n\n";

            for (const auto& ns : namespaces) {
                for (const auto& cls : ns.classes) {
                    ss << GenerateProxyClass(ns, cls) << "\n";
                }
            }

            // Generate agent classes
            for (const auto& ns : namespaces) {
                for (const auto& cls : ns.classes) {
                    ss << GenerateAgentClass(ns, cls) << "\n";
                }
            }

            ss << "} // namespace net\n";
            ss << "} // namespace kai\n";

            // Write output file
            std::ofstream out(outputFile);
            if (!out) {
                std::cerr << "Failed to open output file: " << outputFile
                          << std::endl;
                return false;
            }

            out << ss.str();
            out.close();

            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error generating proxy code: " << e.what()
                      << std::endl;
            return false;
        }
    }
};

// Function to generate network proxy classes from Tau interface definitions
void GenerateNetworkProxy() {
    std::cout << "GenerateNetworkProxy: Starting proxy generation..."
              << std::endl;

    const std::string inputFile = "TestInterface.tau";
    const std::string outputFile = "GeneratedProxy.h";

    ProxyGenerator generator;
    if (generator.ProcessFile(inputFile, outputFile)) {
        std::cout
            << "GenerateNetworkProxy: Successfully generated proxy code in "
            << outputFile << std::endl;
    } else {
        std::cerr << "GenerateNetworkProxy: Failed to generate proxy code"
                  << std::endl;
    }
}

KAI_NET_END
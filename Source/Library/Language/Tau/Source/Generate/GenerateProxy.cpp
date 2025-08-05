#include <KAI/Language/Tau/Generate/GenerateProxy.h>
#include <KAI/Language/Tau/TauParser.h>

using namespace std;

TAU_BEGIN

namespace Generate {

GenerateProxy::GenerateProxy(const char *input, string &output) {
    GenerateProcess::Generate(input, output);
}

bool GenerateProxy::Generate(TauParser const &p, string &output) {
    // Use the base class implementation which handles Module structure properly
    return GenerateProcess::Generate(p, output);
}

string GenerateProxy::Prepend() const {
    stringstream str;
    str << "#include <KAI/Network/ProxyDecl.h>\n";
    str << "#include <KAI/Network/NetworkException.h>\n";
    str << "#include <functional>\n";
    str << "#include <stdexcept>\n";
    str << "#include <string>\n";
    str << "#include <future>\n";
    str << "#include <RakNet/BitStream.h>\n\n";
    return str.str();
}

struct GenerateProxy::ProxyDecl {
    string RootName;
    string ProxyName;

    ProxyDecl(string const &root) : RootName(root) {
        ProxyName = root + "Proxy";
    }

    string ToString() const {
        stringstream decl;
        decl << "class " << ProxyName << ": public ProxyBase";
        return decl.str();
    }
};

void GenerateProxy::AddProxyBoilerplate(ProxyDecl const &proxy) {
    Output() << "using ProxyBase::StreamType;" << EndLine();
    Output() << proxy.ProxyName
             << "(Node &node, NetHandle handle) : ProxyBase(node, handle) { }"
             << EndLine();
    Output() << EndLine();
}

bool GenerateProxy::Namespace(Node const &ns) {
    StartBlock(string("namespace ") + ns.GetToken().Text());
    for (auto const &ch : ns.GetChildren()) {
        switch (ch->GetType()) {
            case TauAstEnumType::Namespace:
                if (!Namespace(*ch)) return false;
                break;

            case TauAstEnumType::Class: {
                // Check if this is actually a struct
                bool isStruct = false;
                for (const auto &child : ch->GetChildren()) {
                    if (child->GetType() == TauAstEnumType::Struct) {
                        isStruct = true;
                        break;
                    }
                }
                // Only process if it's not a struct
                if (!isStruct) {
                    if (!Class(*ch)) return false;
                }
                break;
            }

            case TauAstEnumType::Interface:
                if (!Interface(*ch)) return false;
                break;

            case TauAstEnumType::Struct:
                // Structs don't need proxy generation, just skip
                break;

            case TauAstEnumType::EnumType:
                // Enums don't need proxy generation, just skip
                break;

            default:
                // Don't fail on unknown types, just skip them
                break;
        }
    }

    EndBlock();
    return true;
}

bool GenerateProxy::Class(Node const &cl) {
    auto className = cl.GetToken().Text();

    // Generate documentation comment
    Output() << "/// Network proxy for " << className << " interface" << EndLine();
    Output() << "/// Provides type-safe remote method calls over the network" << EndLine();
    Output() << "/// All methods are synchronous and may throw NetworkException on failure" << EndLine();

    // Generate Proxy class only
    auto proxyDecl = ProxyDecl(className);
    StartBlock(proxyDecl.ToString());
    AddProxyBoilerplate(proxyDecl);

    // Handle class members for proxy
    for (const auto &member : cl.GetChildren()) {
        switch (member->GetType()) {
            case TauAstEnumType::Class:
                if (!Class(*member)) return false;
                break;

            case TauAstEnumType::Property:
                if (!Property(*member)) return false;
                break;

            case TauAstEnumType::Method:
                if (!Method(*member)) return false;
                break;

            case TauAstEnumType::Event:
                if (!Event(*member)) return false;
                break;

            case TauAstEnumType::Interface:
                if (!Interface(*member)) return false;
                break;

            // Skip special nodes like 'Interface' marker nodes
            case TauAstEnumType::Struct:
            case TauAstEnumType::Inherits:
                // Skip these - they're just markers
                break;

            default:
                // Ignore unknown node types for resilience
                break;
        }
    }

    EndBlock();
    return true;
}

bool GenerateProxy::Event(Node const &event) {
    const auto name = event.GetTokenText();
    const auto args = event.GetChild(0)->GetChildren();

    // Generate event registration method
    Output() << "void Register" << name << "Handler(std::function<void(";

    // Generate parameter list for event handler
    bool first = true;
    for (auto const &a : args) {
        if (!first) Output() << ", ";

        auto &ty = a->GetChild(0);
        Output() << ty->GetTokenText();

        first = false;
    }

    Output() << ")> handler)";
    StartBlock();
    Output() << "RegisterEventHandler(\"" << name << "\", handler);";
    EndBlock();
    Output() << EndLine();

    // Generate event unregistration method
    Output() << "void Unregister" << name << "Handler()";
    StartBlock();
    Output() << "UnregisterEventHandler(\"" << name << "\");";
    EndBlock();
    Output() << EndLine();

    return true;
}

bool GenerateProxy::Property(Node const &prop) {
    auto type = prop.GetChild(0)->GetTokenText();
    auto name = prop.GetChild(1)->GetTokenText();

    // Generate property getter
    Output() << ReturnType(type);
    Output() << " " << name << "()";
    StartBlock();
    Output() << "return Fetch<" << type << ">(\"" << name << "\");";
    EndBlock();
    Output() << EndLine();

    // Generate property setter
    Output() << "void Set" << name << "(" << type << " value)";
    StartBlock();
    Output() << "Store(\"" << name << "\", value);";
    EndBlock();
    Output() << EndLine();

    return true;
}

bool GenerateProxy::Method(Node const &method) {
    auto const &returnType = method.GetChild(0)->GetTokenText();
    auto const &args = method.GetChild(1)->GetChildren();
    const auto name = method.GetTokenText();

    MethodDecl(returnType, args, name);
    MethodBody(returnType, args, name);

    Output() << EndLine();

    return true;
}

void GenerateProxy::MethodDecl(const string &returnType,
                               const Node::ChildrenType &args,
                               const string &name) {
    // Add method documentation
    Output() << "/// Remote method call: " << name << EndLine();
    if (!args.empty()) {
        Output() << "/// Parameters:" << EndLine();
        for (auto const &a : args) {
            auto &ty = a->GetChild(0);
            auto &id = a->GetChild(1);
            Output() << "///   @param " << id->GetTokenText() << " " << ty->GetTokenText() << EndLine();
        }
    }
    if (returnType != "void") {
        Output() << "/// @return " << returnType << EndLine();
    }
    Output() << "/// @throws NetworkException on communication failure" << EndLine();
    
    Output() << returnType << " " << name << "(";
    bool first = true;
    for (auto const &a : args) {
        if (!first) Output() << ", ";

        auto &ty = a->GetChild(0);
        auto &id = a->GetChild(1);
        string typeText = ty->GetTokenText();
        
        // Use appropriate parameter passing for different types
        if (typeText == "int" || typeText == "float" || typeText == "bool" || 
            typeText == "double" || typeText == "char") {
            // Pass by value for primitive types
            Output() << typeText << " " << id->GetTokenText();
        } else {
            // Pass by const reference for complex types
            Output() << "const " << typeText << "& " << id->GetTokenText();
        }

        first = false;
    }
    Output() << ")";
}

void GenerateProxy::MethodBody(const string &returnType,
                               const Node::ChildrenType &args,
                               const string &name) {
    StartBlock();

    if (returnType == "void") {
        // For void methods, use _node->Send
        if (!args.empty()) {
            Output() << "RakNet::BitStream args;" << EndLine();
            for (auto const &a : args) {
                Output() << "args << " << a->GetChild(1)->GetTokenText() << ";"
                         << EndLine();
            }
            Output() << "try {" << EndLine();
            Output() << "    _node->Send(\"" << name << "\", args);" << EndLine();
            Output() << "} catch (const std::exception& e) {" << EndLine();
            Output() << "    throw NetworkException(\"Failed to send '" << name << "': \" + std::string(e.what()));" << EndLine();
            Output() << "}" << EndLine();
        } else {
            Output() << "try {" << EndLine();
            Output() << "    _node->Send(\"" << name << "\");" << EndLine();
            Output() << "} catch (const std::exception& e) {" << EndLine();
            Output() << "    throw NetworkException(\"Failed to send '" << name << "': \" + std::string(e.what()));" << EndLine();
            Output() << "}" << EndLine();
        }
    } else {
        // For non-void methods, use _node->SendWithResponse
        if (!args.empty()) {
            Output() << "RakNet::BitStream args;" << EndLine();
            for (auto const &a : args) {
                Output() << "args << " << a->GetChild(1)->GetTokenText() << ";"
                         << EndLine();
            }
            Output() << "try {" << EndLine();
            Output() << "    auto future = _node->SendWithResponse(\"" << name
                     << "\", args);" << EndLine();
            Output() << "    return future.get();" << EndLine();
            Output() << "} catch (const std::exception& e) {" << EndLine();
            Output() << "    throw NetworkException(\"Failed to call '" << name << "': \" + std::string(e.what()));" << EndLine();
            Output() << "}" << EndLine();
        } else {
            Output() << "try {" << EndLine();
            Output() << "    auto future = _node->SendWithResponse(\"" << name
                     << "\");" << EndLine();
            Output() << "    return future.get();" << EndLine();
            Output() << "} catch (const std::exception& e) {" << EndLine();
            Output() << "    throw NetworkException(\"Failed to call '" << name << "': \" + std::string(e.what()));" << EndLine();
            Output() << "}" << EndLine();
        }
    }

    EndBlock();
}

string GenerateProxy::ReturnType(string const &text) const { return text; }

string GenerateProxy::ArgType(string const &text) const {
    return "const " + text + "&";
}

bool GenerateProxy::Interface(Node const &interface) {
    // Interfaces are treated like classes for proxy generation
    return Class(interface);
}

bool GenerateProxy::Struct(Node const &strct) {
    // Structs don't need proxy generation - skip them entirely
    return true;
}

}  // namespace Generate

TAU_END

// EOF
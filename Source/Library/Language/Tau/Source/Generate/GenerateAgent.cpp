#include <KAI/Language/Tau/Generate/GenerateAgent.h>

TAU_BEGIN

namespace Generate {
namespace {
bool IsStdStringType(const std::string &typeText) {
    return typeText == "string" || typeText == "std::string";
}

// Mirrors GenerateProxy.cpp's helper of the same name: the IDL allows a
// method/property return type to be spelled explicitly as "Future<T>". The
// proxy side unwraps this to T before calling Exec<T>/Fetch<T> (since those
// already return Future<T>), so the agent side must unwrap it the same way
// for RegisterMethod<T,...>/RegisterProperty<T> and the implementation's
// own (plain, synchronous) method/property signature to stay consistent
// with what the proxy expects on the wire. Parameter types are never
// unwrapped - a Future<T> parameter genuinely means "pass a Future<T>
// value" (see Node::Invoke / PackInvokeArg).
std::string UnwrapFutureReturnType(const std::string &text) {
    if (text.rfind("Future<", 0) == 0 && !text.empty() && text.back() == '>') {
        return text.substr(7, text.size() - 8);
    }
    return text;
}
}  // namespace

GenerateAgent::GenerateAgent(const char *input, string &output) {
    GenerateProcess::Generate(input, output);
}

bool GenerateAgent::GenerateFromFile(const char *filename, string &output,
                                     string &error) {
    std::ifstream file(filename);
    if (!file) {
        error = string("Could not open file: ") + filename;
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    string contents = buffer.str();
    file.close();

    if (contents.empty()) {
        error = string("File is empty: ") + filename;
        return false;
    }

    GenerateAgent agent(contents.c_str(), output);
    if (agent.failed) {
        error = agent.error;
        return false;
    }

    return true;
}

bool GenerateAgent::Generate(TauParser const &parser, string &output) {
    // Use the base class implementation which handles Module structure properly
    return GenerateProcess::Generate(parser, output);
}

string GenerateAgent::Prepend() const {
    stringstream str;
    str << "#include <KAI/Network/AgentDecl.h>\n";
    // AgentDecl.h -> AgentBase.h only forward-declares Node (to avoid a
    // circular include), so the full Node.h is needed explicitly here for
    // GetNode().RegisterMethod<...>/RegisterProperty<...>/BroadcastEvent.
    str << "#include <KAI/Network/Node.h>\n";
    str << "#include <KAI/Network/NetworkException.h>\n";
    str << "#include <functional>\n";
    str << "#include <memory>\n";
    str << "#include <stdexcept>\n";
    str << "#include <string>\n";
    str << "#include <utility>\n";
    str << "#include <KAI/Core/BinaryStream.h>\n";
    str << "#include <KAI/Network/Future.h>\n";
    str << "\n";
    return str.str();
}

bool GenerateAgent::Namespace(Node const &ns) {
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
                // Structs don't need agent generation, just skip
                break;

            case TauAstEnumType::EnumType:
                // Enums don't need agent generation, just skip
                break;

            default:
                // Don't fail on unknown types, just skip them
                break;
        }
    }

    EndBlock();
    return true;
}

struct GenerateAgent::AgentDecl {
    string RootName;
    string AgentName;

    AgentDecl(string const &root) : RootName(root) {
        AgentName = root + "Agent";
    }

    // AgentBase is a plain (non-template) base - it takes NetHandle
    // attachment care of itself via Node::AttachAgent. The implementation
    // type is held separately as `_impl`, not baked into the base class.
    string ToString() const {
        stringstream decl;
        decl << "class " << AgentName << ": public AgentBase";
        return decl.str();
    }
};

bool GenerateAgent::Class(TauParser::AstNode const &cl) {
    auto className = cl.GetToken().Text();

    // Generate documentation comment
    Output() << "/// Network agent for " << className << " interface"
             << EndLine();
    Output() << "/// Registers method and property handlers with the Node "
                "that dispatch to the implementation"
             << EndLine();

    auto agentDecl = AgentDecl(className);
    StartBlock(agentDecl.ToString());
    Output() << "public:" << EndLine();

    // The constructor itself registers every method/property with the
    // Node (see AddAgentBoilerplate) - that's how a real incoming call
    // reaches the implementation (Node::ProcessFunctionCall dispatches to
    // whatever was registered via RegisterMethod, it does not look for a
    // hand-written Handle_MethodName function).
    AddAgentBoilerplate(agentDecl, cl);

    // Event triggers remain plain public methods the implementation (or
    // its owner) calls directly to broadcast to connected clients.
    for (const auto &member : cl.GetChildren()) {
        if (member->GetType() == TauAstEnumType::Event) {
            GenerateEventTrigger(*member);
        }
    }

    Output() << "private:" << EndLine();
    Output() << "std::shared_ptr<" << agentDecl.RootName << "> _impl;"
             << EndLine();

    EndBlock();
    return true;
}

bool GenerateAgent::Property(TauParser::AstNode const &prop) {
    // Agents don't need property accessors - they handle properties through
    // messages
    return true;
}

bool GenerateAgent::Method(TauParser::AstNode const &method) {
    // Agents don't expose methods directly - they handle them through messages
    return true;
}

std::string GenerateAgent::ArgType(std::string const &text) const {
    return text;
}

std::string GenerateAgent::ReturnType(std::string const &text) const {
    return text;
}

void GenerateAgent::AddAgentBoilerplate(AgentDecl const &agent,
                                        TauParser::AstNode const &cl) {
    // AgentBase(Node&) attaches itself to the Node (via Node::AttachAgent)
    // and owns the resulting NetHandle - there's no separate handle
    // parameter to pass in. The implementation is supplied by the caller
    // and held as _impl; every method/property gets wired to it right here
    // via Node::RegisterMethod/RegisterProperty so that a real incoming
    // call (Node::ProcessFunctionCall / ProcessPropertyGet / SetProperty)
    // has something registered to dispatch to.
    Output() << agent.AgentName << "(Node &node, std::shared_ptr<"
             << agent.RootName << "> impl)" << EndLine();
    Output() << "    : AgentBase(node), _impl(std::move(impl))";
    StartBlock();

    for (const auto &member : cl.GetChildren()) {
        switch (member->GetType()) {
            case TauAstEnumType::Method:
                GenerateHandlerMethod(*member);
                break;
            case TauAstEnumType::Property:
                GenerateHandlerProperty(*member);
                break;
            default:
                break;
        }
    }

    EndBlock();
    Output() << EndLine();
}

void GenerateAgent::GenerateHandlerMethod(TauParser::AstNode const &method) {
    auto const returnType =
        UnwrapFutureReturnType(method.GetChild(0)->GetTokenText());
    auto const &args = method.GetChild(1)->GetChildren();
    const auto name = method.GetTokenText();

    Output() << "/// Registers remote method call: " << name << EndLine();
    Output() << "GetNode().RegisterMethod<" << returnType;
    for (auto const &a : args) {
        auto &ty = a->GetChild(0);
        Output() << ", " << ty->GetTokenText();
    }
    Output() << ">(" << EndLine();
    Output() << "    GetHandle(), \"" << name << "\"," << EndLine();
    Output() << "    std::function<" << returnType << "(";
    {
        bool first = true;
        for (auto const &a : args) {
            if (!first) Output() << ", ";
            Output() << a->GetChild(0)->GetTokenText();
            first = false;
        }
    }
    Output() << ")>(" << EndLine();
    Output() << "        [this](";
    {
        bool first = true;
        for (auto const &a : args) {
            if (!first) Output() << ", ";
            Output() << a->GetChild(0)->GetTokenText() << " "
                     << a->GetChild(1)->GetTokenText();
            first = false;
        }
    }
    Output() << ") {" << EndLine();
    Output() << "            return _impl->" << name << "(";
    {
        bool first = true;
        for (auto const &a : args) {
            if (!first) Output() << ", ";
            Output() << a->GetChild(1)->GetTokenText();
            first = false;
        }
    }
    // A method whose return type is "void" still compiles as
    // `return voidExpr;` inside a lambda returning void, so this line
    // works unchanged regardless of returnType.
    Output() << ");" << EndLine();
    Output() << "        }));" << EndLine();
}

void GenerateAgent::GenerateHandlerProperty(TauParser::AstNode const &prop) {
    auto const type = UnwrapFutureReturnType(prop.GetChild(0)->GetTokenText());
    auto const &name = prop.GetChild(1)->GetTokenText();

    // Mirrors GenerateProxy::Property, which calls Fetch<T>(name) /
    // Store(name, value) - both resolve, over the network, to exactly this
    // registration (Node::ProcessPropertyGet / ProcessPropertySet look up
    // the accessor registered here by name).
    Output() << "/// Registers remote property accessors: " << name
             << EndLine();
    Output() << "GetNode().RegisterProperty<" << type << ">(" << EndLine();
    Output() << "    GetHandle(), \"" << name << "\"," << EndLine();
    Output() << "    [this]() { return _impl->" << name << "(); },"
             << EndLine();
    Output() << "    [this](" << type << " value) { _impl->Set" << name
             << "(value); });" << EndLine();
}

void GenerateAgent::GenerateEventTrigger(TauParser::AstNode const &event) {
    const auto name = event.GetTokenText();
    const auto args = event.GetChild(0)->GetChildren();

    // Generate documentation for event trigger
    Output() << "/// Trigger event: " << name << EndLine();
    Output() << "/// Broadcasts event to all connected clients" << EndLine();
    if (!args.empty()) {
        Output() << "/// Event parameters:" << EndLine();
        for (auto const &a : args) {
            auto &ty = a->GetChild(0);
            auto &id = a->GetChild(1);
            Output() << "///   " << id->GetTokenText() << " ("
                     << ty->GetTokenText() << ")" << EndLine();
        }
    }

    // Generate event trigger method signature
    Output() << "void Trigger" << name << "(";
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

    StartBlock();

    // Serialize event parameters
    if (!args.empty()) {
        Output() << "BinaryStream eventData;" << EndLine();
        for (auto const &a : args) {
            auto &ty = a->GetChild(0);
            auto &id = a->GetChild(1);
            if (IsStdStringType(ty->GetTokenText())) {
                Output()
                    << "kai::net::NetworkSerializer::WriteString(eventData, "
                    << id->GetTokenText() << ");" << EndLine();
            } else {
                Output() << "eventData << " << id->GetTokenText() << ";"
                         << EndLine();
            }
        }
        Output() << "GetNode().BroadcastEvent(\"" << name
                 << "\", eventData);" << EndLine();
    } else {
        Output() << "GetNode().BroadcastEvent(\"" << name << "\");"
                 << EndLine();
    }

    EndBlock();
    Output() << EndLine();
}

bool GenerateAgent::Interface(Node const &interface) {
    // Interfaces are handled the same way as classes in agent generation
    return Class(interface);
}

bool GenerateAgent::Struct(Node const &strct) {
    // Structs don't need agent generation - skip them entirely
    return true;
}

}  // namespace Generate

TAU_END

// EOF

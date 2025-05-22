#include <KAI/Language/Tau/Generate/GenerateProxy.h>
#include <KAI/Language/Tau/TauParser.h>

using namespace std;

TAU_BEGIN

namespace Generate {

GenerateProxy::GenerateProxy(const char *input, string &output) {
    GenerateProcess::Generate(input, output);
}

bool GenerateProxy::Generate(TauParser const &p, string &output) {
    // Be more resilient to parsing errors for code generation
    auto const &root = p.GetRoot();
    
    // Debug: log what we actually got from the parser (commented out for cleaner output)
    // KAI_TRACE_1(string("GenerateProxy received root node type: ") + TauAstEnumType::ToString(root->GetType()));
    // KAI_TRACE_1(string("GenerateProxy received root node children count: ") + std::to_string(root->GetChildren().size()));
    // for (size_t i = 0; i < root->GetChildren().size(); ++i) {
    //     auto child = root->GetChildren()[i];
    //     KAI_TRACE_1(string("Child ") + std::to_string(i) + " type: " + TauAstEnumType::ToString(child->GetType()));
    // }
    
    if (root->GetType() != TauAstEnumType::Module) {
        KAI_TRACE_WARN_1("Expected a Module but continuing anyway");
        // Continue with generation using GenerateProcess's implementation
        return GenerateProcess::Generate(p, output);
    }

    // Handle all children, even if they're not all namespaces
    bool processed = false;
    for (const auto &ch : root->GetChildren()) {
        if (ch->GetType() == TauAstEnumType::Namespace) {
            if (!Namespace(*ch)) {
                KAI_TRACE_WARN_1("Namespace processing failed, but continuing");
            }
            processed = true;
        } else if (ch->GetType() == TauAstEnumType::Class) {
            // Handle class without namespace by wrapping in Default namespace
            StartBlock("namespace Default");
            if (!Class(*ch)) {
                KAI_TRACE_WARN_1("Class processing failed, but continuing");
            }
            EndBlock();
            processed = true;
        }
        // Skip other node types but continue processing
    }

    // If nothing was processed, create an empty default namespace
    if (!processed) {
        KAI_TRACE_WARN_1("No valid namespaces or classes found, creating empty default namespace");
        StartBlock("namespace Default");
        EndBlock();
    }

    stringstream str;
    str << Prepend() << "\n" << Output().str() << std::ends;
    output = str.str();
    return !Failed;
}

string GenerateProxy::Prepend() const {
    return string("#include <KAI/Network/ProxyDecl.h>\n\n");
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

struct GenerateProxy::AgentDecl {
    string RootName;
    string AgentName;

    AgentDecl(string const &root) : RootName(root) {
        AgentName = root + "Agent";
    }

    string ToString() const {
        stringstream decl;
        decl << "class " << AgentName << ": public AgentBase<" << RootName << ">";
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

            case TauAstEnumType::Class:
                if (!Class(*ch)) return false;
                break;

            case TauAstEnumType::Interface:
                if (!Interface(*ch)) return false;
                break;

            default:
                KAI_TRACE_ERROR_1("Parser failed to fail");
                Fail("[Internal] Unexpected %s in namespace",
                     TauAstEnumType::ToString(ch->GetType()));
                break;
        }
    }

    EndBlock();
    return true;
}

bool GenerateProxy::Class(Node const &cl) {
    auto className = cl.GetToken().Text();
    
    // Generate Proxy class
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
    Output() << EndLine();
    
    // Generate Agent class
    auto agentDecl = AgentDecl(className);
    StartBlock(agentDecl.ToString());
    AddAgentBoilerplate(agentDecl);

    // Generate handler methods for each method in the class
    for (const auto &member : cl.GetChildren()) {
        if (member->GetType() == TauAstEnumType::Method) {
            GenerateHandlerMethod(*member);
        }
    }

    EndBlock();
    return true;
}

bool GenerateProxy::Interface(Node const &interface) {
    // Interfaces are handled the same way as classes in proxy generation
    return Class(interface);
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
    Output() << returnType << " " << name << "(";
    bool first = true;
    for (auto const &a : args) {
        if (!first) Output() << ", ";

        auto &ty = a->GetChild(0);
        auto &id = a->GetChild(1);
        Output() << "const " << ty->GetTokenText() << "& " << id->GetTokenText();

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
                Output() << "args << " << a->GetChild(1)->GetTokenText() << ";" << EndLine();
            }
            Output() << "_node->Send(\"" << name << "\", args);" << EndLine();
        } else {
            Output() << "_node->Send(\"" << name << "\");" << EndLine();
        }
    } else {
        // For non-void methods, use _node->SendWithResponse
        if (!args.empty()) {
            Output() << "RakNet::BitStream args;" << EndLine();
            for (auto const &a : args) {
                Output() << "args << " << a->GetChild(1)->GetTokenText() << ";" << EndLine();
            }
            Output() << "auto future = _node->SendWithResponse(\"" << name << "\", args);" << EndLine();
        } else {
            Output() << "auto future = _node->SendWithResponse(\"" << name << "\");" << EndLine();
        }
        Output() << "return future.get();" << EndLine();
    }

    EndBlock();
}

string GenerateProxy::ReturnType(string const &text) const {
    return text;
}

string GenerateProxy::ArgType(string const &text) const { return "const " + text + "&"; }

void GenerateProxy::AddAgentBoilerplate(AgentDecl const &agent) {
    Output() << agent.AgentName
             << "(Node &node, NetHandle handle) : AgentBase(node, handle) { }"
             << EndLine();
    Output() << EndLine();
}

void GenerateProxy::GenerateHandlerMethod(Node const &method) {
    auto const &returnType = method.GetChild(0)->GetTokenText();
    auto const &args = method.GetChild(1)->GetChildren();
    const auto name = method.GetTokenText();

    // Generate the Handle_MethodName signature
    Output() << "void Handle_" << name << "(RakNet::BitStream& bs, RakNet::SystemAddress& sender)";
    StartBlock();

    // Deserialize parameters from BitStream
    for (auto const &a : args) {
        auto &ty = a->GetChild(0);
        auto &id = a->GetChild(1);
        Output() << ty->GetTokenText() << " " << id->GetTokenText() << ";" << EndLine();
        Output() << "bs >> " << id->GetTokenText() << ";" << EndLine();
    }

    // Call the implementation method
    if (returnType != "void") {
        Output() << returnType << " result = _impl->" << name << "(";
    } else {
        Output() << "_impl->" << name << "(";
    }

    // Pass arguments
    bool first = true;
    for (auto const &a : args) {
        if (!first) Output() << ", ";
        auto &id = a->GetChild(1);
        Output() << id->GetTokenText();
        first = false;
    }
    Output() << ");" << EndLine();

    // Send back result for non-void methods
    if (returnType != "void") {
        Output() << "RakNet::BitStream response;" << EndLine();
        Output() << "response << result;" << EndLine();
        Output() << "_node->SendResponse(sender, response);" << EndLine();
    }

    EndBlock();
    Output() << EndLine();
}

}  // namespace Generate

TAU_END

// EOF
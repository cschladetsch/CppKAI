#include <KAI/Language/Tau/Generate/GenerateProxy.h>
#include <KAI/Language/Tau/TauParser.h>

using namespace std;

TAU_BEGIN

namespace Generate {

GenerateProxy::GenerateProxy(const char *input, string &output) {
    GenerateProcess::Generate(input, output);
}

bool GenerateProxy::Generate(TauParser const &p, string &output) {
    auto const &root = p.GetRoot();
    if (root->GetType() != TauAstEnumType::Module)
        return Fail("Expected a Module");

    for (const auto &ch : root->GetChildren()) {
        if (ch->GetType() != TauAstEnumType::Namespace)
            return Fail("Namespace expected");

        if (!Namespace(*ch)) return false;
    }

    stringstream str;
    str << Prepend() << "\n" << Output().str() << ends;
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
        decl << "struct " << ProxyName << ": ProxyBase";
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
    auto decl = ProxyDecl(cl.GetToken().Text());

    StartBlock(decl.ToString());
    AddProxyBoilerplate(decl);

    // Handle class members
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
    Output() << ReturnType(returnType) << " " << name << "(";
    bool first = true;
    for (auto const &a : args) {
        if (!first) Output() << ", ";

        auto &ty = a->GetChild(0);
        auto &id = a->GetChild(1);
        Output() << ArgType(ty->GetTokenText()) << " " << id->GetTokenText();

        first = false;
    }
    Output() << ")";
}

static string ReturnLead(string const &rt, string const &name) {
    stringstream str;
    str << "return Exec<" << rt << ">(\"" << name << "\"";
    return str.str();
}

void GenerateProxy::MethodBody(const string &returnType,
                               const Node::ChildrenType &args,
                               const string &name) {
    StartBlock();
    const auto ret = ReturnLead(returnType, name);
    if (!args.empty()) {
        Output() << "StreamType args;" << EndLine();
        Output() << "args";
        for (auto const &a : args) {
            Output() << " << " << a->GetChild(1)->GetTokenText();
        }
        Output() << ";" << EndLine();
        Output() << ret << ", args);";
    } else {
        Output() << ret << ");";
    }

    Output() << EndLine();
    EndBlock();
}

string GenerateProxy::ReturnType(string const &text) const {
    return string("Future<") + text + ">";
}

string GenerateProxy::ArgType(string const &text) const { return text; }

}  // namespace Generate

TAU_END

// EOF
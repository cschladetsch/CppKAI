#include <KAI/Language/Tau/Generate/GenerateAgent.h>

TAU_BEGIN

namespace Generate {
GenerateAgent::GenerateAgent(const char *input, string &output) {
    GenerateProcess::Generate(input, output);
}

bool GenerateAgent::Generate(TauParser const &parser, string &output) {
    return GenerateProcess::Generate(parser, output);
}

string GenerateAgent::Prepend() const {
    return string("#include <KAI/Network/AgentDecl.h>\n\n");
}

bool GenerateAgent::Namespace(Node const &cl) { return true; }

struct GenerateAgent::Decl {
    string RootName;
    string AgentName;

    Decl(string const &root) : RootName(root) { AgentName = root + "Agent"; }

    string ToString() const {
        stringstream decl;
        decl << "struct " << AgentName << ": AgentBase<" << RootName << ">";
        return decl.str();
    }
};

bool GenerateAgent::Class(TauParser::AstNode const &cl) {
    auto decl = Decl(cl.GetToken().Text());

    StartBlock(decl.ToString());
    AddAgentBoilerplate(decl);

    GenerateProcess::Class(cl);

    EndBlock();
    return true;
}

bool GenerateAgent::Property(TauParser::AstNode const &prop) {
    auto type = prop.GetChild(0)->GetTokenText();
    auto name = prop.GetChild(1)->GetTokenText();

    // Generate getter
    Output() << ReturnType(type);
    Output() << " " << name << "()";
    StartBlock();
    Output() << "return GetLocalValue<" << type << ">(\"" << name << "\");";
    EndBlock();
    Output() << EndLine();

    // Generate setter
    Output() << "void Set" << name << "(" << type << " value)";
    StartBlock();
    Output() << "SetLocalValue(\"" << name << "\", value);";
    EndBlock();
    Output() << EndLine();

    return true;
}

bool GenerateAgent::Method(TauParser::AstNode const &method) {
    auto const &returnType = method.GetChild(0)->GetTokenText();
    auto const &args = method.GetChild(1)->GetChildren();
    const auto name = method.GetTokenText();

    MethodDecl(returnType, args, name);
    MethodBody(returnType, args, name);

    Output() << EndLine();

    return true;
}

std::string GenerateAgent::ArgType(std::string const &text) const {
    return text;
}

std::string GenerateAgent::ReturnType(std::string const &text) const {
    return text;
}

void GenerateAgent::AddAgentBoilerplate(Decl const &agent) {
    Output() << agent.AgentName
             << "(Node &node, NetHandle handle) : AgentBase(node, handle) { }"
             << EndLine();
    Output() << EndLine();
}

void GenerateAgent::MethodDecl(const string &returnType,
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

void GenerateAgent::MethodBody(const string &returnType,
                               const Node::ChildrenType &args,
                               const string &name) {
    StartBlock();

    // Build arguments for the call
    if (!args.empty()) {
        Output() << "// Process method arguments" << EndLine();
        for (auto const &a : args) {
            auto &id = a->GetChild(1);
            Output() << "// Validate " << id->GetTokenText() << EndLine();
        }
    }

    // Execute local method implementation
    Output() << "// Execute local method implementation" << EndLine();
    if (returnType != "void") {
        Output() << returnType << " result = ";
    }

    Output() << "LocalCall_" << name << "(";

    // Pass arguments
    bool first = true;
    for (auto const &a : args) {
        if (!first) Output() << ", ";
        auto &id = a->GetChild(1);
        Output() << id->GetTokenText();
        first = false;
    }
    Output() << ");" << EndLine();

    // Return result if needed
    if (returnType != "void") {
        Output() << "return result;" << EndLine();
    }

    EndBlock();
}
}  // namespace Generate

TAU_END

// EOF

#include <KAI/Language/Tau/Generate/GenerateAgent.h>

TAU_BEGIN

namespace Generate {
GenerateAgent::GenerateAgent(const char *input, string &output) {
    GenerateProcess::Generate(input, output);
}

bool GenerateAgent::Generate(TauParser const &parser, string &output) {
    // Make agent generation more resilient to parsing errors
    auto const &root = parser.GetRoot();
    
    // Validate the root node but continue even if it's not what we expect
    if (root->GetType() != TauAstEnumType::Module) {
        KAI_TRACE_WARN_1("Expected a Module but continuing anyway");
    }
    
    // Process all nodes, even if they're malformed
    bool processed = false;
    
    // Process all node types that we understand
    for (const auto &ch : root->GetChildren()) {
        if (ch->GetType() == TauAstEnumType::Namespace) {
            StartBlock(string("namespace ") + ch->GetToken().Text());
            
            // Process all classes in the namespace
            for (const auto &nsChild : ch->GetChildren()) {
                if (nsChild->GetType() == TauAstEnumType::Class) {
                    Class(*nsChild);
                    processed = true;
                }
            }
            
            EndBlock();
            processed = true;
        } else if (ch->GetType() == TauAstEnumType::Class) {
            // Handle class without namespace by wrapping in Default namespace
            StartBlock("namespace Default");
            Class(*ch);
            EndBlock();
            processed = true;
        }
    }
    
    // If nothing was processed, create a default empty namespace
    if (!processed) {
        KAI_TRACE_WARN_1("No valid namespaces or classes found, creating empty default namespace");
        StartBlock("namespace Default");
        EndBlock();
    }
    
    // Format the output
    stringstream str;
    str << Prepend() << "\n" << Output().str() << std::ends;
    output = str.str();
    return !Failed;
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
        decl << "class " << AgentName << ": public AgentBase<" << RootName << ">";
        return decl.str();
    }
};

bool GenerateAgent::Class(TauParser::AstNode const &cl) {
    auto decl = Decl(cl.GetToken().Text());

    StartBlock(decl.ToString());
    AddAgentBoilerplate(decl);

    // Generate handler methods for each method in the class
    for (const auto &member : cl.GetChildren()) {
        if (member->GetType() == TauAstEnumType::Method) {
            GenerateHandlerMethod(*member);
        }
    }

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

void GenerateAgent::GenerateHandlerMethod(TauParser::AstNode const &method) {
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

#include <KAI/Language/Tau/Generate/GenerateAgent.h>

TAU_BEGIN

namespace Generate {
GenerateAgent::GenerateAgent(const char *input, string &output) {
    GenerateProcess::Generate(input, output);
}

bool GenerateAgent::Generate(TauParser const &parser, string &output) {
    // Use the base class implementation which handles Module structure properly
    return GenerateProcess::Generate(parser, output);
}

string GenerateAgent::Prepend() const {
    stringstream str;
    str << "#include <KAI/Network/AgentDecl.h>\n";
    str << "#include <KAI/Network/NetworkException.h>\n";
    str << "#include <stdexcept>\n";
    str << "#include <string>\n";
    str << "#include <RakNet/BitStream.h>\n\n";
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

    string ToString() const {
        stringstream decl;
        decl << "class " << AgentName << ": public AgentBase<" << RootName
             << ">";
        return decl.str();
    }
};

bool GenerateAgent::Class(TauParser::AstNode const &cl) {
    auto className = cl.GetToken().Text();
    
    // Generate documentation comment
    Output() << "/// Network agent for " << className << " interface" << EndLine();
    Output() << "/// Handles incoming network requests and dispatches to implementation" << EndLine();
    Output() << "/// All handler methods deserialize parameters and call implementation" << EndLine();

    auto agentDecl = AgentDecl(className);
    StartBlock(agentDecl.ToString());
    AddAgentBoilerplate(agentDecl);

    // Generate handler methods for each method and event in the class
    for (const auto &member : cl.GetChildren()) {
        switch (member->GetType()) {
            case TauAstEnumType::Method:
                GenerateHandlerMethod(*member);
                break;
            case TauAstEnumType::Event:
                GenerateEventTrigger(*member);
                break;
            case TauAstEnumType::Property:
                // Properties are handled through messages, no special generation needed
                break;
            default:
                // Skip other member types
                break;
        }
    }

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

void GenerateAgent::AddAgentBoilerplate(AgentDecl const &agent) {
    Output() << agent.AgentName
             << "(Node &node, NetHandle handle) : AgentBase(node, handle) { }"
             << EndLine();
    Output() << EndLine();
}

void GenerateAgent::GenerateHandlerMethod(TauParser::AstNode const &method) {
    auto const &returnType = method.GetChild(0)->GetTokenText();
    auto const &args = method.GetChild(1)->GetChildren();
    const auto name = method.GetTokenText();

    // Generate documentation for handler method
    Output() << "/// Handler for remote method call: " << name << EndLine();
    Output() << "/// Deserializes parameters from BitStream and calls implementation" << EndLine();
    if (!args.empty()) {
        Output() << "/// Parameters deserialized from network:" << EndLine();
        for (auto const &a : args) {
            auto &ty = a->GetChild(0);
            auto &id = a->GetChild(1);
            Output() << "///   " << id->GetTokenText() << " (" << ty->GetTokenText() << ")" << EndLine();
        }
    }
    if (returnType != "void") {
        Output() << "/// Sends " << returnType << " response back to sender" << EndLine();
    }

    // Generate the Handle_MethodName signature
    Output() << "void Handle_" << name
             << "(RakNet::BitStream& bs, RakNet::SystemAddress& sender)";
    StartBlock();

    // Deserialize parameters from BitStream
    for (auto const &a : args) {
        auto &ty = a->GetChild(0);
        auto &id = a->GetChild(1);
        Output() << ty->GetTokenText() << " " << id->GetTokenText() << ";"
                 << EndLine();
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
            Output() << "///   " << id->GetTokenText() << " (" << ty->GetTokenText() << ")" << EndLine();
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
        Output() << "RakNet::BitStream eventData;" << EndLine();
        for (auto const &a : args) {
            auto &id = a->GetChild(1);
            Output() << "eventData << " << id->GetTokenText() << ";" << EndLine();
        }
        Output() << "_node->BroadcastEvent(\"" << name << "\", eventData);" << EndLine();
    } else {
        Output() << "_node->BroadcastEvent(\"" << name << "\");" << EndLine();
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

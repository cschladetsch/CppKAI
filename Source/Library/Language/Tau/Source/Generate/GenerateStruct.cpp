#include <KAI/Language/Tau/Generate/GenerateStruct.h>
#include <KAI/Language/Tau/TauParser.h>

using namespace std;

TAU_BEGIN

namespace Generate {

GenerateStruct::GenerateStruct(const char *input, string &output) {
    GenerateProcess::Generate(input, output);
}

bool GenerateStruct::Generate(TauParser const &p, string &output) {
    // Use base class implementation
    return GenerateProcess::Generate(p, output);
}

string GenerateStruct::Prepend() const {
    return string("#include <cstdint>\n\n");
}

bool GenerateStruct::Namespace(Node const &ns) {
    KAI_TRACE_2("GenerateStruct::Namespace called with name: ", ns.GetToken().Text());
    KAI_TRACE_2("Namespace has children: ", static_cast<int>(ns.GetChildren().size()));
    
    StartBlock(string("namespace ") + ns.GetToken().Text());
    for (auto const &ch : ns.GetChildren()) {
        switch (ch->GetType()) {
            case TauAstEnumType::Namespace:
                if (!Namespace(*ch)) return false;
                break;

            case TauAstEnumType::Class: {
                // Check if this is actually a struct (has Struct child node)
                bool isStruct = false;
                for (const auto &child : ch->GetChildren()) {
                    if (child->GetType() == TauAstEnumType::Struct) {
                        isStruct = true;
                        break;
                    }
                }
                
                if (isStruct) {
                    if (!Struct(*ch)) return false;
                } else {
                    if (!Class(*ch)) return false;
                }
                break;
            }

            case TauAstEnumType::Struct:
                if (!Struct(*ch)) return false;
                break;

            case TauAstEnumType::Interface:
                // Treat interfaces as structs
                if (!Interface(*ch)) return false;
                break;

            default:
                return Fail(string("Unknown node type in Namespace: ") + TauAstEnumType::ToString(ch->GetType()));
        }
    }

    EndBlock();
    return true;
}

bool GenerateStruct::Class(Node const &cl) {
    // Classes in GenerateStruct context are treated as structs
    return Struct(cl);
}

bool GenerateStruct::Interface(Node const &interface) {
    // Interfaces in GenerateStruct context are treated as structs
    return Struct(interface);
}

bool GenerateStruct::Struct(Node const &strct) {
    auto structName = strct.GetToken().Text();
    
    // Skip empty struct markers
    if (structName.empty()) {
        KAI_TRACE_1("Skipping empty struct marker node");
        return true;
    }
    
    KAI_TRACE_2("GenerateStruct::Struct called with name: ", structName);
    KAI_TRACE_2("Struct node has children: ", static_cast<int>(strct.GetChildren().size()));
    
    // Debug: dump all children
    for (size_t i = 0; i < strct.GetChildren().size(); ++i) {
        auto child = strct.GetChildren()[i];
        KAI_TRACE_3("Child", static_cast<int>(i), TauAstEnumType::ToString(child->GetType()));
        KAI_TRACE_2("  Token text: ", child->GetToken().Text());
        if (child->GetType() == TauAstEnumType::Struct) {
            KAI_TRACE_2("  Struct child has num children: ", static_cast<int>(child->GetChildren().size()));
        }
    }
    
    // Generate plain struct
    StartBlock(string("struct ") + structName);
    
    // Handle struct members
    for (const auto &member : strct.GetChildren()) {
        // Skip the struct marker node
        if (member->GetType() == TauAstEnumType::Struct && member->GetToken().Text().empty()) {
            KAI_TRACE_1("Skipping struct marker node");
            continue;
        }
        
        switch (member->GetType()) {
            case TauAstEnumType::Property:
                if (!Property(*member)) return false;
                break;

            case TauAstEnumType::Method:
                if (!Method(*member)) return false;
                break;

            case TauAstEnumType::Struct:
                // Nested struct with a name
                if (!Struct(*member)) return false;
                break;
                
            case TauAstEnumType::Class:
                // Nested class
                if (!Struct(*member)) return false;
                break;

            case TauAstEnumType::Interface:
                // Skip interface marker node (similar to struct marker)
                continue;

            default:
                return Fail(string("Unknown node type in Struct: ") + TauAstEnumType::ToString(member->GetType()));
        }
    }

    EndBlock();
    Output() << ";";
    return true;
}

bool GenerateStruct::Property(Node const &prop) {
    auto type = prop.GetChild(0)->GetTokenText();
    auto name = prop.GetChild(1)->GetTokenText();

    // Generate simple member variable
    Output() << type << " " << name << ";" << EndLine();
    
    return true;
}

bool GenerateStruct::Method(Node const &method) {
    auto const &returnType = method.GetChild(0)->GetTokenText();
    auto const &args = method.GetChild(1)->GetChildren();
    const auto name = method.GetTokenText();

    // Generate method declaration
    Output() << returnType << " " << name << "(";
    
    bool first = true;
    for (auto const &a : args) {
        if (!first) Output() << ", ";

        auto &ty = a->GetChild(0);
        auto &id = a->GetChild(1);
        Output() << ty->GetTokenText() << " " << id->GetTokenText();

        first = false;
    }
    
    Output() << ");" << EndLine();
    
    return true;
}

string GenerateStruct::ReturnType(string const &text) const {
    return text;
}

string GenerateStruct::ArgType(string const &text) const { 
    return text; 
}

bool GenerateStruct::Module(TauParser const &p) {
    KAI_TRACE_1("GenerateStruct::Module called");
    auto const &root = p.GetRoot();
    
    KAI_TRACE_2("Root has children: ", static_cast<int>(root->GetChildren().size()));

    if (root->GetChildren().empty()) {
        return Fail("Module has no children");
    }

    for (const auto &ch : root->GetChildren()) {
        KAI_TRACE_2("Processing child type: ", TauAstEnumType::ToString(ch->GetType()));
        if (ch->GetType() == TauAstEnumType::Module) {
            // Handle module node
            for (const auto &moduleChild : ch->GetChildren()) {
                if (moduleChild->GetType() == TauAstEnumType::Namespace) {
                    if (!Namespace(*moduleChild)) {
                        return false;
                    }
                } else if (moduleChild->GetType() == TauAstEnumType::Class) {
                    // Check if this is actually a struct (has Struct child node)
                    bool isStruct = false;
                    for (const auto &child : moduleChild->GetChildren()) {
                        if (child->GetType() == TauAstEnumType::Struct) {
                            isStruct = true;
                            break;
                        }
                    }
                    
                    // Directly handle class/struct without namespace
                    StartBlock("namespace Default");
                    if (isStruct) {
                        if (!Struct(*moduleChild)) {
                            return false;
                        }
                    } else {
                        if (!Class(*moduleChild)) {
                            return false;
                        }
                    }
                    EndBlock();
                }
            }
        } else if (ch->GetType() == TauAstEnumType::Namespace) {
            // Directly handle namespace node
            if (!Namespace(*ch)) {
                return false;
            }
        } else if (ch->GetType() == TauAstEnumType::Class) {
            // Check if this is actually a struct (has Struct child node)
            bool isStruct = false;
            for (const auto &child : ch->GetChildren()) {
                if (child->GetType() == TauAstEnumType::Struct) {
                    isStruct = true;
                    break;
                }
            }
            
            // Directly handle class/struct without namespace
            StartBlock("namespace Default");
            if (isStruct) {
                if (!Struct(*ch)) {
                    return false;
                }
            } else {
                if (!Class(*ch)) {
                    return false;
                }
            }
            EndBlock();
        } else {
            return Fail(string("Unknown node type in Module: ") + TauAstEnumType::ToString(ch->GetType()));
        }
    }

    return true;
}

}  // namespace Generate

TAU_END

// EOF
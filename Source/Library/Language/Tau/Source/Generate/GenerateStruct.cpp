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
                // Skip interfaces - GenerateStruct only generates actual
                // structs
                break;

            case TauAstEnumType::EnumType:
                // Skip enums - GenerateStruct only generates actual structs
                break;

            default:
                // Skip unknown types instead of failing
                KAI_TRACE_WARN_1(
                    "Skipping unknown node type in Namespace: " +
                    string(TauAstEnumType::ToString(ch->GetType())));
                break;
        }
    }

    EndBlock();
    return true;
}

bool GenerateStruct::Class(Node const &cl) {
    // GenerateStruct treats all classes as structs
    return Struct(cl);
}

bool GenerateStruct::Interface(Node const &interface) {
    // Skip interfaces - GenerateStruct only generates actual structs
    return true;
}

bool GenerateStruct::Struct(Node const &strct) {
    auto structName = strct.GetToken().Text();

    // Skip empty struct markers
    if (structName.empty()) {
        return true;
    }

    // Generate plain struct
    StartBlock(string("struct ") + structName);

    // Handle struct members
    for (const auto &member : strct.GetChildren()) {
        // Skip the struct marker node
        if (member->GetType() == TauAstEnumType::Struct &&
            member->GetToken().Text().empty()) {
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
                return Fail(string("Unknown node type in Struct: ") +
                            TauAstEnumType::ToString(member->GetType()));
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

string GenerateStruct::ReturnType(string const &text) const { return text; }

string GenerateStruct::ArgType(string const &text) const { return text; }

}  // namespace Generate

TAU_END

// EOF
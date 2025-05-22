#include <KAI/Core/File.h>
#include <KAI/Language/Tau/Generate/GenerateProcess.h>

#include <fstream>

using namespace std;

TAU_BEGIN

namespace Generate {
bool GenerateProcess::Generate(const char *input, string &output) {
    const auto parser = Parse(input);
    return parser && Generate(*parser, output);
}

shared_ptr<TauParser> GenerateProcess::Parse(const char *input) const {
    Registry r;
    auto lex = make_shared<TauLexer>(input, r);
    if (!lex->Process()) {
        Fail(lex->Error);
        return nullptr;
    }

    KAI_TRACE_1(lex->Print());

    auto parser = make_shared<TauParser>(r);
    // Use Structure::Module for top-level parsing
    if (!parser->Process(lex, Structure::Module)) {
        Fail(parser->Error);
        return nullptr;
    }

    return parser;
}

bool GenerateProcess::Generate(TauParser const &p, string &output) {
    if (!Module(p)) return false;

    stringstream str;
    str << CommonPrepend() << Prepend() << str_.str() << ends;
    output = str.str();
    return !Failed;
}

string GenerateProcess::CommonPrepend() {
    stringstream str;
    str << "// auto-generated on " << "Tuesday" << "; modify at own risk\n";
    return str.str();
}

bool GenerateProcess::Module(TauParser const &p) {
    auto const &root = p.GetRoot();

    // Debug: log what we actually got from the parser (commented out for cleaner output)
    // KAI_TRACE_1(string("Module() received root node type: ") + TauAstEnumType::ToString(root->GetType()));
    // KAI_TRACE_1(string("Module() received root node children count: ") + std::to_string(root->GetChildren().size()));
    // for (size_t i = 0; i < root->GetChildren().size(); ++i) {
    //     auto child = root->GetChildren()[i];
    //     KAI_TRACE_1(string("Child ") + std::to_string(i) + " type: " + TauAstEnumType::ToString(child->GetType()));
    // }

    // Be more resilient with code generation - even if the module is empty
    if (root->GetChildren().empty()) {
        KAI_TRACE_WARN_1("Empty module found, creating empty default namespace");
        StartBlock("namespace Default");
        EndBlock();
        return true;
    }

    // Start with a default namespace if none exists
    bool handledAnyNodes = false;

    for (const auto &ch : root->GetChildren()) {
        if (ch->GetType() == TauAstEnumType::Module) {
            // Handle module node
            for (const auto &moduleChild : ch->GetChildren()) {
                if (moduleChild->GetType() == TauAstEnumType::Namespace) {
                    if (!Namespace(*moduleChild)) {
                        // Continue even if namespace processing fails
                        KAI_TRACE_WARN_1("Namespace processing failed, but continuing");
                    }
                    handledAnyNodes = true;
                } else if (moduleChild->GetType() == TauAstEnumType::Class) {
                    // Directly handle class without namespace
                    StartBlock("namespace Default");
                    if (!Class(*moduleChild)) {
                        // Continue even if class processing fails
                        KAI_TRACE_WARN_1("Class processing failed, but continuing");
                    }
                    EndBlock();
                    handledAnyNodes = true;
                } else {
                    // Log but continue - be more resilient to errors
                    KAI_TRACE_WARN_1("Unexpected node type in module, but continuing");
                }
            }
        } else if (ch->GetType() == TauAstEnumType::Namespace) {
            // Directly handle namespace node
            if (!Namespace(*ch)) {
                // Continue even if namespace processing fails
                KAI_TRACE_WARN_1("Namespace processing failed, but continuing");
            }
            handledAnyNodes = true;
        } else if (ch->GetType() == TauAstEnumType::Class) {
            // Directly handle class without namespace
            StartBlock("namespace Default");
            if (!Class(*ch)) {
                // Continue even if class processing fails
                KAI_TRACE_WARN_1("Class processing failed, but continuing");
            }
            EndBlock();
            handledAnyNodes = true;
        } else {
            // Log but continue - be more resilient to errors
            KAI_TRACE_WARN_1("Unexpected node type at root, but continuing");
        }
    }

    if (!handledAnyNodes) {
        KAI_TRACE_WARN_1("No valid Module, Namespace, or Class nodes found, creating empty default namespace");
        StartBlock("namespace Default");
        EndBlock();
        return true;
    }

    return true;
}

bool GenerateProcess::Namespace(Node const &ns) {
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
                return Fail("[Internal] Unexpected %s in namespace",
                            TauAstEnumType::ToString(ch->GetType()));
        }
    }

    EndBlock();
    return true;
}

bool GenerateProcess::Class(Node const &cl) {
    for (const auto &member : cl.GetChildren()) {
        switch (member->GetType()) {
            case TauAstEnumType::Class:
                return Class(*member);

            case TauAstEnumType::Property:
                if (!Property(*member)) return false;
                break;

            case TauAstEnumType::Method:
                if (!Method(*member)) return false;
                break;

            default:
                return Fail("Invalid class member: %s",
                            TauAstEnumType::ToString(member->GetType()));
        }
    }

    return true;
}

bool GenerateProcess::Property(Node const &prop) { return true; }

bool GenerateProcess::Method(Node const &method) { return true; }

string GenerateProcess::Prepend() const { return ""; }

stringstream &GenerateProcess::StartBlock(const string &name) {
    str_ << name << EndLine() << '{';
    indentation_++;
    str_ << EndLine();
    return str_;
}

string GenerateProcess::EndLine() const {
    stringstream s;
    s << "\n" << string(indentation_, '\t');
    return s.str();
}

void GenerateProcess::EndBlock() {
    indentation_--;
    str_ << EndLine() << '}';
}
}  // namespace Generate

TAU_END

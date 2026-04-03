#pragma once

#include <KAI/Language/Tau/Generate/GenerateProcess.h>

TAU_BEGIN

namespace Generate {
struct GenerateStruct : GenerateProcess {
    using GenerateProcess::Node;

    GenerateStruct(const char *input, string &output);

   protected:
    bool Generate(TauParser const &p, string &output) override;
    string Prepend() const override;
    bool Namespace(Node const &ns) override;
    bool Class(Node const &cl) override;
    bool Interface(Node const &interface) override;
    bool Property(Node const &prop) override;
    bool Method(Node const &method) override;
    bool Struct(Node const &strct) override;
    string ArgType(string const &text) const override;
    string ReturnType(string const &text) const override;
};
}  // namespace Generate

TAU_END

// EOF
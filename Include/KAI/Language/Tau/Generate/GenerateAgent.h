#pragma once

#include <KAI/Language/Tau/Generate/GenerateProcess.h>

TAU_BEGIN

namespace Generate {
struct GenerateAgent : GenerateProcess {
    using GenerateProcess::Node;

    GenerateAgent(const char *input, string &output);

   protected:
    bool Generate(TauParser const &parser, string &output) override;

    string Prepend() const override;
    bool Namespace(Node const &cl) override;
    bool Class(Node const &cl) override;
    bool Property(Node const &prop) override;
    bool Method(Node const &method) override;
    string ArgType(string const &text) const override;
    string ReturnType(string const &text) const override;

   private:
    struct Decl;
    void AddAgentBoilerplate(Decl const &agent);
    void MethodBody(const string &returnType, const Node::ChildrenType &args,
                    const string &name);
    void MethodDecl(const string &returnType, const Node::ChildrenType &args,
                    const string &name);
    void GenerateHandlerMethod(Node const &method);
};
}  // namespace Generate

TAU_END

// EOF

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
    bool Interface(Node const &interface) override;
    bool Struct(Node const &strct) override;
    string ArgType(string const &text) const override;
    string ReturnType(string const &text) const override;

   private:
    struct AgentDecl;
    void AddAgentBoilerplate(AgentDecl const &agent);
    void GenerateHandlerMethod(Node const &method);
    void GenerateEventTrigger(Node const &event);
};
}  // namespace Generate

TAU_END

// EOF

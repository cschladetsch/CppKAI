#pragma once

#include <KAI/Language/Tau/Generate/GenerateProcess.h>

TAU_BEGIN

namespace Generate {
struct GenerateProxy : GenerateProcess {
    using GenerateProcess::Node;

    GenerateProxy(const char *input, string &output);

   protected:
    bool Generate(TauParser const &p, string &output) override;

    string Prepend() const override;
    bool Namespace(Node const &ns) override;
    bool Class(Node const &cl) override;
    bool Property(Node const &prop) override;
    bool Method(Node const &method) override;
    bool Event(Node const &event);
    bool Interface(Node const &interface) override;
    bool Struct(Node const &strct) override;
    string ArgType(string const &text) const override;
    string ReturnType(string const &text) const override;

   private:
    struct ProxyDecl;
    void AddProxyBoilerplate(ProxyDecl const &);
    void MethodBody(const string &returnType, const Node::ChildrenType &args,
                    const string &name);
    void MethodDecl(const string &returnType, const Node::ChildrenType &args,
                    const string &name);
};
}  // namespace Generate

TAU_END

// EOF

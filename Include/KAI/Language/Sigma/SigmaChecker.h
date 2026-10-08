#pragma once

#include <KAI/Core/Config/Base.h>
#include <KAI/Language/Sigma/SigmaAstNode.h>

#include <deque>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

KAI_BEGIN

struct Registry;

struct SigmaType;
using SigmaTypePtr = std::shared_ptr<const SigmaType>;

/// A Sigma static type.
///
///   any  void  bool  int  float  str  List[T]  Map[str, V]  fun(T, ...) -> R
///   <registered class name>   (native C++ type, resolved through the Registry)
///   fun[T, ...](...) -> R        (a template function; T appears as Var)
///
/// Generics are invariant. The only implicit conversion is int -> float, which
/// the compiler makes explicit in the generated Rho.
struct SigmaType {
    enum class Kind { Any, Void, Bool, Int, Float, Str, List, Map, Fun, Native, Var };

    Kind kind = Kind::Any;
    std::vector<SigmaTypePtr> args;       // List: [T]. Map: [K, V]. Fun: parameters.
    SigmaTypePtr result;                  // Fun only
    int native = 0;                       // Native: Type::Number
    std::string name;                     // Native: class name. Var: type parameter name
    std::vector<std::string> typeParams;  // Fun only: non-empty for a template function

    [[nodiscard]] bool Is(Kind k) const { return kind == k; }
    [[nodiscard]] bool IsNumeric() const { return kind == Kind::Int || kind == Kind::Float; }
    [[nodiscard]] std::string ToString() const;

    static SigmaTypePtr Of(Kind k);
    static SigmaTypePtr ListOf(SigmaTypePtr element);
    static SigmaTypePtr MapOf(SigmaTypePtr key, SigmaTypePtr value);
    static SigmaTypePtr FunOf(std::vector<SigmaTypePtr> params, SigmaTypePtr result,
                              std::vector<std::string> typeParams = {});
    static SigmaTypePtr VarOf(std::string name);

    [[nodiscard]] bool IsTemplate() const { return kind == Kind::Fun && !typeParams.empty(); }
    static SigmaTypePtr NativeOf(int typeNumber, std::string name);

    static bool Same(const SigmaTypePtr &a, const SigmaTypePtr &b);
    /// May a value of type `from` be stored where `to` is expected?
    static bool Assignable(const SigmaTypePtr &from, const SigmaTypePtr &to);
};

struct SigmaDiagnostic {
    int line = 0;    // zero-based
    int column = 0;  // zero-based
    std::string message;

    /// "line:column: message", one-based.
    [[nodiscard]] std::string ToString() const;
};

/// Static checker for Sigma. Every name must be declared before use, either
/// with a type (`x: T = e`) or by its first assignment (`x = e`, whose type is
/// then fixed). Function parameters are always typed; a function without
/// `-> T` returns void.
class SigmaChecker {
   public:
    using NodePtr = SigmaAstNodePtr;
    using Tok = SigmaTokenEnumType;
    using Ast = SigmaAstNodeEnumType;
    using Kind = SigmaType::Kind;

    /// The registry resolves native class names and the signatures of their
    /// reflected methods. It may be null.
    explicit SigmaChecker(Registry *reg = nullptr) : reg_(reg) {}

    /// Top-level names that persist between programs, e.g. REPL inputs.
    struct Global {
        SigmaTypePtr type;
        bool function = false;
        NodePtr definition;  // template functions only: needed to check later instantiations
        // What the definition's tokens read their text from (its lexer).
        // Tokens hold a raw pointer to it, so it must outlive the AST.
        std::shared_ptr<const void> source;
    };
    using Globals = std::unordered_map<std::string, Global>;

    /// Names declared by earlier programs in the same session. They may be
    /// redeclared, but only with the same type.
    void SetSession(Globals globals) { session_ = std::move(globals); }

    /// What this program's tokens read their text from (its lexer). Kept by
    /// GetGlobals() alongside any template defined here, so the session can
    /// check later calls to it after the program's lexer would have gone.
    void SetSource(std::shared_ptr<const void> source) { source_ = std::move(source); }

    /// All top-level names after Check(), including the session's.
    [[nodiscard]] Globals GetGlobals() const;

    /// Returns true if the program is well typed.
    bool Check(const NodePtr &program);

    [[nodiscard]] const std::vector<SigmaDiagnostic> &GetDiagnostics() const { return diagnostics_; }

    /// Expressions whose int value must be widened to float in the output.
    [[nodiscard]] const std::unordered_set<const SigmaAstNode *> &GetWidened() const { return widened_; }

    /// Type of a top-level name after Check(), or null.
    [[nodiscard]] SigmaTypePtr GetGlobalType(const std::string &name) const;

   private:
    struct Binding {
        SigmaTypePtr type;
        bool function = false;
        bool session = false;  // declared by an earlier program
    };
    using Scope = std::unordered_map<std::string, Binding>;

    Registry *reg_ = nullptr;
    Globals session_;
    std::shared_ptr<const void> source_;
    std::deque<Scope> scopes_;  // deque: growing it never moves existing scopes
    std::vector<SigmaDiagnostic> diagnostics_;
    std::unordered_set<const SigmaAstNode *> widened_;
    std::vector<NodePtr> functions_;
    const SigmaAstNode *function_ = nullptr;  // function whose body is being checked
    SigmaTypePtr result_;                     // its result type
    const SigmaAstNode *tailCall_ = nullptr;  // the one `f(...)!` allowed in that body, if any
    int loops_ = 0;

    // Template functions. Each call infers the type arguments and queues an
    // instantiation; Check() then checks the body once per distinct set.
    using TypeArgs = std::unordered_map<std::string, SigmaTypePtr>;
    struct Instance {
        NodePtr fun;
        TypeArgs args;
        std::string key;  // e.g. "max[int]"
    };
    std::unordered_map<std::string, NodePtr> templates_;  // name -> definition
    std::vector<Instance> instances_;
    std::unordered_set<std::string> instanceKeys_;
    TypeArgs typeArgs_;  // type parameters in scope while resolving types
    std::string note_;   // appended to diagnostics, e.g. "in max[int]"

    // statements
    void Statements(const NodePtr &block, bool topLevel);
    void Statement(const NodePtr &node, bool topLevel);
    void Declaration(const NodePtr &node);
    void Assignment(const NodePtr &node);
    void ForEach(const NodePtr &node);
    void Return(const NodePtr &node);
    void FunctionBody(const NodePtr &fun, const TypeArgs &typeArgs = {});
    void CheckTemplates();
    void Condition(const NodePtr &node, const char *what);
    static bool AlwaysReturns(const NodePtr &node);

    // expressions
    SigmaTypePtr Expr(const NodePtr &node, const SigmaTypePtr &expected = nullptr);
    SigmaTypePtr Name(const NodePtr &node);
    SigmaTypePtr Binary(const NodePtr &node);
    SigmaTypePtr Unary(const NodePtr &node);
    SigmaTypePtr Ternary(const NodePtr &node, const SigmaTypePtr &expected);
    SigmaTypePtr Call(const NodePtr &node);
    SigmaTypePtr PlainCall(const NodePtr &node);
    SigmaTypePtr TemplateCall(const NodePtr &node, const std::string &name, const SigmaTypePtr &sig);
    void TailCall(const NodePtr &node, const SigmaTypePtr &type);
    const SigmaAstNode *TailCallIn(const NodePtr &body);
    SigmaTypePtr MethodCall(const NodePtr &call, const NodePtr &member, const NodePtr &args);
    SigmaTypePtr Arguments(const NodePtr &at, const std::string &what, const std::vector<SigmaTypePtr> &params,
                           const NodePtr &args);
    SigmaTypePtr Member(const NodePtr &node);
    SigmaTypePtr Index(const NodePtr &node);
    SigmaTypePtr List(const NodePtr &node, const SigmaTypePtr &expected);
    SigmaTypePtr Map(const NodePtr &node, const SigmaTypePtr &expected);
    SigmaTypePtr Unify(const std::vector<NodePtr> &nodes, const std::vector<SigmaTypePtr> &types,
                       const NodePtr &at, const char *what);
    SigmaTypePtr BinaryResult(const NodePtr &at, Tok::Enum op, const SigmaTypePtr &a, const SigmaTypePtr &b);

    // types
    SigmaTypePtr Resolve(const NodePtr &typeNode, bool allowVoid = false);
    SigmaTypePtr Signature(const NodePtr &fun);
    SigmaTypePtr FromTypeNumber(int typeNumber) const;
    static std::vector<std::string> TypeParamNames(const NodePtr &fun);
    static SigmaTypePtr Substitute(const SigmaTypePtr &type, const TypeArgs &args);
    static bool HasVars(const SigmaTypePtr &type);
    static bool Match(const SigmaTypePtr &param, const SigmaTypePtr &arg, TypeArgs &bound);

    // scopes and diagnostics
    Binding *Lookup(const std::string &name);
    void Bind(const std::string &name, SigmaTypePtr type, bool function = false);
    /// True if `name` may be (re)declared with `type` here; reports otherwise.
    bool Declarable(const NodePtr &at, const std::string &name, const SigmaTypePtr &type, bool function);
    void Report(const NodePtr &at, const std::string &message);
    void Report(const SigmaToken &at, const std::string &message);
    /// Check assignability and record int -> float widening of `node`.
    bool Convert(const NodePtr &node, const SigmaTypePtr &from, const SigmaTypePtr &to, const std::string &context);
};

KAI_END

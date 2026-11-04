#pragma once

#include <KAI/Console/Console.h>
#include <KAI/Core/Detail/Function.h>
#include <KAI/Language/Sigma/SigmaTranslator.h>

#include <format>
#include <functional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

KAI_BEGIN

/// Thrown when a program given to a SigmaHost does not compile. what() holds
/// every diagnostic, one "line:column: message" per line.
struct SigmaError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

/// The C++ side of Sigma: embed it in an application, give it C++ functions,
/// types and values, run programs, and read typed results back.
///
///     SigmaHost sigma(console);
///     sigma.Bind("clamp", [](int v, int lo, int hi) { return std::clamp(v, lo, hi); });
///     sigma.Set("limit", 10);
///     sigma.Run("fun double_it(n: int) -> int\n    return n * 2");
///     int r = sigma.Eval<int>("clamp(double_it(limit), 0, 15)");   // 15
///     auto f = sigma.Callback<int(int)>("double_it");              // a std::function
///
/// Everything Bind() and Set() publish goes in the console's tree, so Sigma
/// type-checks calls and uses against the C++ signature before anything runs.
/// Definitions persist between Run() and Eval() calls.
class SigmaHost {
   public:
    explicit SigmaHost(Console &console) : console_(console), translator_(console.GetRegistry()) {
        auto root = console_.GetTree().GetRoot();
        // Make the root searchable, as Rho expects of a host's published names.
        if (root.Exists()) console_.GetTree().AddSearchPath(root);
    }

    /// Publish a C++ function (free function, lambda or std::function) under
    /// `name`. Parameter and result types are taken from its signature, so
    /// they must be types KAI knows: bool, int, float, String, or a registered
    /// class. A lambda returning a registered class is a constructor.
    template <class F>
    void Bind(const char *name, F &&fn) {
        BindImpl(name, std::function(std::forward<F>(fn)));
    }

    /// Publish a registered class under its own name as a zero-argument
    /// constructor: `p = Point()`. Bind() a factory for constructors that take
    /// arguments.
    template <class T>
    void Construct(const char *name) {
        BindImpl(name, std::function<T()>([] { return T(); }));
    }

    /// Publish a value (or a registered object) as a name Sigma can read.
    template <class T>
    void Set(const char *name, const T &value) {
        Root().SetChild(Label(name), console_.GetRegistry().New(value));
    }

    /// Read a published or top-level name back.
    template <class T>
    T Get(const char *name) {
        Object found = console_.GetTree().Resolve(Label(name));
        if (!found.Exists()) throw SigmaError(std::format("no value named '{}'", name));
        if (!found.IsType<T>()) throw SigmaError(std::format("'{}' does not have the requested type", name));
        return ConstDeref<T>(found);
    }

    /// Compile and run a program. Throws SigmaError if it does not compile.
    void Run(const std::string &code) { Execute(code); }

    /// Compile and run a program, and return the value it leaves on the stack.
    /// Throws SigmaError on a compile error or a result of another type.
    template <class T>
    T Eval(const std::string &code) {
        auto data = console_.GetExecutor()->GetDataStack();
        data->Clear();
        Execute(code);
        if (data->Empty()) throw SigmaError("the program left no value");
        Object top = data->Top();
        if (!top.IsType<T>()) throw SigmaError("the program's value does not have the requested type");
        return ConstDeref<T>(top);
    }

    /// Call a Sigma (or bound) function by name with C++ arguments.
    template <class R, class... Args>
    R Call(const std::string &name, Args &&...args) {
        std::string call = name + "(";
        const char *sep = "";
        ((call += sep + Literal(args), sep = ", "), ...);
        call += ")";
        if constexpr (std::is_void_v<R>) {
            Execute(call + "\n");
        } else {
            return Eval<R>(call);
        }
    }

    /// A Sigma function as a C++ callable: `sigma.Callback<int(int)>("f")(3)`.
    template <class Sig>
    std::function<Sig> Callback(const std::string &name) {
        return MakeCallback(name, static_cast<std::function<Sig> *>(nullptr));
    }

    /// Forget declarations remembered between Run() calls (not the tree).
    void ResetSession() { translator_.ResetSession(); }

   private:
    Console &console_;
    SigmaTranslator translator_;

    Object Root() { return console_.GetTree().GetRoot(); }

    void Execute(const std::string &code) {
        auto cont = translator_.Translate(code.c_str(), Structure::Program);
        if (translator_.failed || !cont.Exists()) {
            std::string all;
            for (auto const &e : translator_.GetErrors()) all += e + "\n";
            if (all.empty()) all = translator_.error;
            throw SigmaError(all);
        }
        console_.Execute(cont);
    }

    template <class R, class... Args>
    void BindImpl(const char *name, std::function<R(Args...)> fn) {
        auto root = Root();
        AddFunction(root, std::move(fn), Label(name));
    }

    static std::string Literal(int v) { return std::to_string(v); }
    static std::string Literal(float v) { return std::to_string(v); }
    static std::string Literal(double v) { return Literal(static_cast<float>(v)); }
    static std::string Literal(bool v) { return v ? "true" : "false"; }
    static std::string Literal(const String &v) { return Literal(v.StdString()); }
    static std::string Literal(const std::string &v) {
        if (v.find('"') != std::string::npos) throw SigmaError("a string argument cannot contain a double quote");
        return "\"" + v + "\"";
    }
    static std::string Literal(const char *v) { return Literal(std::string(v)); }

    template <class R, class... Args>
    std::function<R(Args...)> MakeCallback(const std::string &name, std::function<R(Args...)> *) {
        return [this, name](Args... args) -> R { return Call<R>(name, args...); };
    }
};

KAI_END

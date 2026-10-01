#pragma once

#include <KAI/Language/Common/TranslatorBase.h>
#include <KAI/Language/Sigma/SigmaChecker.h>
#include <KAI/Language/Sigma/SigmaParser.h>

#include <string>
#include <vector>

KAI_BEGIN

/// Sigma: statically typed KAI. Sigma source is lexed and parsed with the
/// Language/Common framework, type checked by SigmaChecker, then lowered to
/// Rho source, which RhoTranslator turns into a Pi continuation.
///
/// Any type error fails translation; nothing runs. The generated Rho is
/// fully parenthesised and makes int -> float widening explicit, so it does
/// not depend on Rho's operator precedence.
class SigmaTranslator : public TranslatorBase<SigmaParser> {
   public:
    using Parent = TranslatorBase<SigmaParser>;

    explicit SigmaTranslator(Registry &reg) : Parent(reg) {}

    Pointer<Continuation> Translate(const char *text, Structure st) override;

    /// Check `text` and produce Rho without running RhoTranslator. Returns
    /// false on any lexical, syntax or type error; see GetErrors().
    bool Compile(const char *text);

    /// The Rho produced by the last successful Compile() or Translate().
    [[nodiscard]] const std::string &GetRho() const { return rho_; }

    /// "line:column: message" for each problem found by the last call.
    [[nodiscard]] const std::vector<std::string> &GetErrors() const { return errors_; }

    /// Top-level names from earlier successful compilations are remembered,
    /// so a REPL can declare `x` on one line and use it on the next.
    void ResetSession() { session_.clear(); }

   protected:
    // Sigma lowers to Rho text rather than appending operations itself.
    void TranslateNode(AstNodePtr) override {}

   private:
    std::string rho_;
    std::vector<std::string> errors_;
    SigmaChecker::Globals session_;
};

KAI_END

#pragma once

#include <KAI/Language/Common/TranslatorBase.h>
#include <KAI/Language/Rho/RhoParser.h>

#include <concepts>
#include <format>
#include <ranges>

KAI_BEGIN

// Concept to ensure AST node compatibility
template <typename T>
concept AstNodeCompatible = requires(T node) {
    { node->GetType() } -> std::convertible_to<int>;
    { node->GetChildren() } -> std::ranges::range;
    { node->Text() } -> std::convertible_to<std::string>;
};

class RhoTranslator : public TranslatorBase<RhoParser> {
   public:
    using Parent = TranslatorBase<RhoParser>;
    using Parser = Parent::Parser;
    using TokenNode = Parent::TokenNode;
    using AstNode = Parent::AstNode;
    using TokenEnum = Parent::TokenEnum;
    using AstNodeEnum = Parent::AstEnum;
    using AstNodePtr = Parent::AstNodePtr;

    RhoTranslator(const RhoTranslator&) = delete;
    RhoTranslator& operator=(const RhoTranslator&) = delete;
    RhoTranslator(RhoTranslator&&) =
        delete;  // Base class move constructor is deleted
    RhoTranslator& operator=(RhoTranslator&&) = default;

    explicit RhoTranslator(Registry& r) : Parent(r) {}
    virtual ~RhoTranslator() = default;

    // Override to make Rho handle direct evaluation of expressions
    // This addresses the issue where expressions are unnecessarily wrapped in
    // continuations
    [[nodiscard]] Pointer<Continuation> Translate(const char* text,
                                                  Structure st) override;

   protected:
    void TranslateNode(AstNodePtr node) override;
    using Parent::reg_;

    // Helper method is now implemented in TranslatorBase

   private:
    void TranslateToken(AstNodePtr node);
    void TranslateFunction(AstNodePtr node);
    void TranslateBlock(AstNodePtr node);
    void TranslateBinaryOp(AstNodePtr node, Operation::Type op);
    void TranslateCall(AstNodePtr node);
    void TranslatePathname(AstNodePtr node);
    void TranslateIndex(AstNodePtr node);
    void TranslateIf(AstNodePtr node);
    void TranslateWhile(AstNodePtr node);
    void TranslateFor(AstNodePtr node);
    void TranslateDoWhile(AstNodePtr node);
    void TranslateForEach(AstNodePtr node);
    void TranslatePiBlock(AstNodePtr node);
    void TranslateList(AstNodePtr node);
    void TranslateMap(AstNodePtr node);

    // Helper method to convert token types to operation types
    Operation::Type TokenToOperation(RhoTokenEnumType::Enum tokenType);
};

KAI_END

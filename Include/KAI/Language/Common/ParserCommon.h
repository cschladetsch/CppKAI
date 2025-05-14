#pragma once

#include <KAI/Language/Common/AstNodeBase.h>
#include <KAI/Language/Common/ParserBase.h>
#include <KAI/Language/Common/Process.h>
#include <KAI/Language/Common/ProcessCommon.h>
#include <KAI/Language/Common/Structure.h>

KAI_BEGIN

// common for all parsers.
// iterate over a stream of tokens to produce an abstract syntax tree_
template <class ELexer, class AstEnumStruct>
class ParserCommon : public ProcessCommon {
   public:
    typedef ELexer Lexer;
    typedef typename Lexer::Token TokenNode;
    typedef typename Lexer::TokenEnumType TokenEnumType;
    typedef typename TokenNode::Enum TokenEnum;
    typedef typename AstEnumStruct::Enum AstEnum;
    typedef AstNodeBase<TokenNode, AstEnumStruct> AstNode;
    typedef std::shared_ptr<AstNode> AstNodePtr;

    explicit ParserCommon(Registry &r) : ProcessCommon(r) {
        current = 0;
        indent = 0;
        lexer.reset();
    }

    virtual bool Process(std::shared_ptr<Lexer> lex, Structure st) = 0;

    const std::string &GetError() const { return error; }
    AstNodePtr GetRoot() const { return root; }
    bool Process();

    template <class T>
    Pointer<T> New(T const &val) {
        return reg_->New<T>(val);
    }

    bool Run(Structure st) {
        try {
            Process(st);
        } catch (Exception::Base &e) {
            if (!Failed)
                Fail(Lexer::CreateErrorMessage(Current(), "%s", e.ToString()));
        } catch (std::exception &f) {
            if (!Failed)
                Fail(Lexer::CreateErrorMessage(Current(), "%s", f.what()));
        } catch (...) {
            if (!Failed)
                Fail(Lexer::CreateErrorMessage(Current(), "internal error"));
        }

        return !Failed;
    }

    std::string PrintTree() const {
        std::stringstream str;
        PrintTree(str, 0, root);
        return str.str();
    }

    std::string ToString() const { return root->ToString(); }

   protected:
    void PrintTree(std::ostream &str, int level, AstNodePtr root) const {
        auto val = root->ToString();
        if (val.empty()) return;
        std::string indent(4 * level, ' ');
        str << indent << val.c_str() << std::endl;
        for (auto const &ch : root->GetChildren()) {
            PrintTree(str, level + 1, ch);
        }
    }

    std::vector<TokenNode> tokens;
    std::vector<AstNodePtr> stack;
    size_t current;
    AstNodePtr root;
    std::string error;
    int indent;
    std::shared_ptr<Lexer> lexer;

   protected:
    bool Has() const { return current < tokens.size(); }

    bool Push(AstNodePtr node) {
        if (node) {
            stack.push_back(node);
            return true;
        }

        return false;
    }

    bool Append(Object Q) {
        if (Empty()) {
            return false;
        }
        Top()->Children.push_back(
            std::make_shared<AstNode>(AstEnum::Object, Q));
        return true;
    }

    AstNodePtr Pop() {
        if (stack.empty()) {
            // MUST CreateError("Internal Error: Parse stack empty");
            KAI_THROW_0(EmptyStack);
        }

        auto last = stack.back();
        stack.pop_back();

        return last;
    }

    AstNodePtr Top() { return stack.back(); }

    bool PushConsume() {
        Push(NewNode(Consume()));
        return true;
    }

    TokenNode const &Next() {
        // First check if tokens vector is empty
        if (tokens.empty()) {
            KAI_TRACE_ERROR_1(Fail("No tokens to process in Next()"));
            KAI_THROW_1(LogicError, "No tokens to process");
        }

        // Increment token index
        ++current;

        // Check if the new index is valid
        if (current >= tokens.size()) {
            KAI_TRACE_ERROR_1(Fail("Next token index out of range"));
            KAI_THROW_1(LogicError, "Next token index out of range");
        }

        return tokens[current];
    }

    TokenNode const &Last() {
        // Check if tokens vector is empty
        if (tokens.empty()) {
            KAI_TRACE_ERROR_1(Fail("No tokens to process in Last()"));
            KAI_THROW_1(LogicError, "No tokens to process in Last()");
        }

        // Check if we can access the previous token
        if (current <= 0) {
            KAI_TRACE_ERROR_1(Fail("No previous token available"));
            KAI_THROW_1(LogicError, "No previous token available");
        }

        return tokens[current - 1];
    }

    TokenNode const &Current() const {
        // First check if tokens vector is empty to avoid range check error
        if (tokens.empty()) {
            KAI_TRACE_ERROR_1(Fail("No tokens to process"));
            KAI_THROW_1(LogicError, "No tokens to process");
        }

        if (current >= tokens.size()) {
            KAI_TRACE_ERROR_1(Fail("Token index out of range"));
            KAI_THROW_1(LogicError, "Token index out of range");
        }

        return tokens[current];
    }

    bool Current(TokenNode node) const {
        if (current >= tokens.size()) {
            return false;
        }

        return tokens[current] == node;
    }

    bool Empty() const { return current >= tokens.size(); }

    TokenNode const &Peek() const {
        // Check if tokens vector is empty
        if (tokens.empty()) {
            KAI_TRACE_ERROR_1(Fail("No tokens to process in Peek()"));
            KAI_THROW_1(LogicError, "No tokens to process in Peek()");
        }

        if (current + 1 >= tokens.size()) {
            KAI_TRACE_ERROR() << "Unexpected end of tokens stream";
            KAI_THROW_1(LogicError, "Peek: No more tokens in stream");
        }

        return tokens[current + 1];
    }

    bool PeekConsume(TokenEnum ty) {
        if (Peek().type == ty) {
            Consume();
            return true;
        }

        return false;
    }

    bool CurrentIs(TokenEnum ty) const { return Current().type == ty; }

    bool PeekIs(TokenEnum ty) const { return Peek().type == ty; }

    bool Consume(TokenEnum ty) {
        if (Current().type == ty) {
            Consume();
            return true;
        }

        return false;
    }

    TokenNode const &Consume() {
        if (current == tokens.size()) {
            KAI_TRACE_ERROR_1(Fail("Unexpected end of file"));
            KAI_THROW_1(LogicError, "Expected something");
        }

        return tokens[current++];
    }

    bool Try(std::vector<TokenEnum> const &types) {
        for (auto ty : types) {
            if (Current().type == ty) return true;
        }
        return false;
    }

    bool Try(TokenEnum type) {
        // Make sure there are tokens to examine and current index is in bounds
        if (tokens.empty() || current >= tokens.size()) {
            return false;  // No tokens or current out of bounds
        }
        return tokens[current].type == type;
    }

    AstNodePtr Expect(TokenEnum type) {
        TokenNode tok = Current();
        if (tok.type != type) {
            Fail(Lexer::CreateErrorMessage(tok, "Expected %s, have %s",
                                           TokenEnumType::ToString(type),
                                           TokenEnumType::ToString(tok.type)));
            return nullptr;
        }

        Next();
        return std::make_shared<AstNode>(Last());
    }

    AstNodePtr NewNode(AstEnum t) { return std::make_shared<AstNode>(t); }
    AstNodePtr NewNode(AstEnum e, TokenNode const &t) const {
        return std::make_shared<AstNode>(e, t);
    }
    AstNodePtr NewNode(TokenNode const &t) {
        return std::make_shared<AstNode>(t);
    }
};

KAI_END

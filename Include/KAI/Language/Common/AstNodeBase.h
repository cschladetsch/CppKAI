#pragma once

#include <memory>
#include <sstream>
#include <string>
#include <vector>

// #include <KAI/Core/Config/Base.h>
#include <KAI/Core/Object.h>
#include <KAI/Language/Common/AstNodeBase.h>

KAI_BEGIN

// A common AST Node, given the Tokens to use, and the
// enumeration type Enum that indicates what type of AST it is
template <class EToken, class AstEnumType>
class AstNodeBase {
   public:
    typedef EToken Token;
    typedef AstNodeBase<Token, AstEnumType> AstNode;
    typedef std::shared_ptr<AstNode> AstNodePtr;
    typedef typename AstEnumType::Enum Enum;
    typedef AstNodeBase<Token, AstEnumType> Self;
    typedef std::shared_ptr<Self> Child;
    typedef std::vector<Child> ChildrenType;

    AstNodeBase() : astType_((Enum)0) {}
    AstNodeBase(Enum e) : astType_(e) {}
    AstNodeBase(Enum e, Token t) : astType_(e), token_(t) {}
    AstNodeBase(Token const &t) : astType_(AstEnumType::TokenType), token_(t) {}

    const Child &GetChild(size_t n) const { return GetChildren()[n]; }
    const ChildrenType &GetChildren() const { return children_; }
    Enum GetType() const { return astType_; }

    const Token &GetToken() const { return token_; }
    std::string GetTokenText() const { return token_.Text(); }

    std::string ToString() const {
        std::stringstream out;
        out << AstEnumType::ToString(astType_) << ": " << token_.ToString()
            << std::ends;
        // out << token_.ToString() << std::ends;
        return out.str();
    }

    std::string Text() const { return token_.Text(); }

    bool Add(AstNodePtr node) {
        if (!node) {
            return false;
        }
        children_.push_back(node);
        return true;
    }

    bool Add(Enum type, Object content) {
        children_.push_back(std::make_shared<Self>(type, content));
        return true;
    }

    bool Add(Token const &tok) {
        Add(std::make_shared<Self>(tok));
        return true;
    }

    friend std::ostream &operator<<(std::ostream &out, Self const &node) {
        return out << node.ToString();
    }

    size_t NumChildren() const { return children_.size(); }

   protected:
    Enum astType_;
    Token token_;
    ChildrenType children_;

   private:
    void PrintTree(std::ostream &out, int level, Self const &self) {
        out << ToString() << std::endl;
        std::string indent(4, ' ');
        for (const auto &ch : GetChildren()) {
            out << indent << ch << std::endl;
            PrintTree(out, level + 1, ch);
        }
    }
};

KAI_END

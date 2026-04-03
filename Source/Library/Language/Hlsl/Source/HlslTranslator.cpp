#include <KAI/Core/BuiltinTypes.h>
#include <KAI/Executor/Operation.h>
#include <KAI/Language/Hlsl/HlslTranslator.h>

#include <boost/lexical_cast.hpp>

using namespace std;

KAI_BEGIN

void HlslTranslator::TranslateToken(AstNodePtr node) {
    switch (node->GetToken().type) {
        case TokenEnum::OpenParan:
            for (auto ch : node->GetChildren()) TranslateNode(ch);
            return;

        case TokenEnum::Not:
            TranslateNode(node->GetChild(0));
            AppendOp(Operation::LogicalNot);
            return;

        case TokenEnum::True:
            AppendOp(Operation::True);
            return;

        case TokenEnum::False:
            AppendOp(Operation::False);
            return;

            // case TokenEnum::Assert:
            //     TranslateNode(node->GetChild(0));
            //     AppendOp(Operation::Assert);
            //     return;

        case TokenEnum::While:
            TranslateWhile(node);
            return;

        case TokenEnum::DivAssign:
            TranslateBinaryOp(node, Operation::DivEquals);
            return;

        case TokenEnum::MulAssign:
            TranslateBinaryOp(node, Operation::MulEquals);
            return;

        case TokenEnum::MinusAssign:
            TranslateBinaryOp(node, Operation::MinusEquals);
            return;

        case TokenEnum::PlusAssign:
            TranslateBinaryOp(node, Operation::PlusEquals);
            return;

        case TokenEnum::Assign:
            TranslateBinaryOp(node, Operation::Store);
            return;

            /*    case TokenEnum::Lookup:
                    AppendOp(Operation::Lookup);
                    return;

                case TokenEnum::Self:
                    AppendOp(Operation::This);
                    return*/
            ;

        case TokenEnum::NotEquiv:
            TranslateBinaryOp(node, Operation::NotEquiv);
            return;

        case TokenEnum::Equiv:
            TranslateBinaryOp(node, Operation::Equiv);
            return;

        case TokenEnum::Less:
            TranslateBinaryOp(node, Operation::Less);
            return;

        case TokenEnum::Greater:
            TranslateBinaryOp(node, Operation::Greater);
            return;

        case TokenEnum::GreaterEquiv:
            TranslateBinaryOp(node, Operation::GreaterOrEquiv);
            return;

        case TokenEnum::LessEquiv:
            TranslateBinaryOp(node, Operation::LessOrEquiv);
            return;

        case TokenEnum::Minus:
            TranslateBinaryOp(node, Operation::Minus);
            return;

        case TokenEnum::Plus:
            TranslateBinaryOp(node, Operation::Plus);
            return;

        case TokenEnum::Mul:
            TranslateBinaryOp(node, Operation::Multiply);
            return;

        case TokenEnum::Divide:
            TranslateBinaryOp(node, Operation::Divide);
            return;

        case TokenEnum::Or:
            TranslateBinaryOp(node, Operation::LogicalOr);
            return;

        case TokenEnum::And:
            TranslateBinaryOp(node, Operation::LogicalAnd);
            return;

        case TokenEnum::Int:
            Append(New<int>(boost::lexical_cast<int>(node->GetTokenText())));
            return;

        case TokenEnum::Float:
            Append(
                New<float>(boost::lexical_cast<float>(node->GetTokenText())));
            return;

        case TokenEnum::String:
            Append(New<String>(node->Text()));
            return;

        case TokenEnum::Ident:
            Append(New<Label>(Label(node->Text())));
            return;

            // case TokenEnum::Pathname:
            //     Append(New<Pathname>(Pathname(node->Text())));
            //     return;

            // case TokenEnum::Yield:
            //     //for (auto ch : node->Children)
            //     //    Translate(ch);
            //     //AppendNewOp(Operation::PushContext);
            //     KAI_NOT_IMPLEMENTED();
            //     return;

        case TokenEnum::Return:
            for (auto ch : node->GetChildren()) TranslateNode(ch);
            AppendOp(Operation::Return);
            return;
    }

    Fail("Unsupported node %s", node->ToString().c_str());
    KAI_TRACE_ERROR_1(Error);
    KAI_NOT_IMPLEMENTED();
}

void HlslTranslator::TranslateBinaryOp(AstNodePtr node, Operation::Type op) {
    TranslateNode(node->GetChild(0));
    TranslateNode(node->GetChild(1));

    AppendNew(Operation(op));
}

// void HlslTranslator::TranslatePathname(AstNodePtr node)
//{
//     Pathname::Elements elements;
//     typedef Pathname::Element El;
//
//     for (auto ch : node->GetChildren())
//     {
//         switch (ch->GetToken().type)
//         {
//         case HlslTokenEnumType::Quote:
//             elements.push_back(El::Quote);
//             break;
//         case HlslTokenEnumType::Sep:
//             elements.push_back(El::Separator);
//             break;
//         case HlslTokenEnumType::Ident:
//             elements.push_back(Label(ch->GetTokenText()));
//             break;
//         }
//     }
//
//     AppendNew(Pathname(move(elements)));
// }

void HlslTranslator::TranslateNode(AstNodePtr node) {
    if (!node) {
        Failed = true;
        return;
    }

    switch (node->GetType()) {
        case AstEnum::Pathname:
            TranslateToken(node);
            return;

        case AstEnum::IndexOp:
            TranslateBinaryOp(node, Operation::Index);
            return;

        case AstEnum::GetMember:
            TranslateBinaryOp(node, Operation::GetProperty);
            return;

        case AstEnum::TokenType:
            TranslateToken(node);
            return;

        case AstEnum::Assignment:
            // like a binary op, but argument order is reversed
            TranslateNode(node->GetChild(1));
            TranslateNode(node->GetChild(0));
            AppendNew<Operation>(Operation(Operation::Store));
            return;

        case AstEnum::Call:
            TranslateCall(node);
            return;

        case AstEnum::Conditional:
            TranslateIf(node);
            return;

        case AstEnum::Block:
            PushNew();
            for (auto st : node->GetChildren()) TranslateNode(st);
            Append(Pop());
            return;

        case AstEnum::List:
            KAI_NOT_IMPLEMENTED();
            return;

        case AstEnum::For:
            TranslateFor(node);
            return;

        case AstEnum::Function:
            TranslateFunction(node);
            return;

        case AstEnum::Program:
            for (auto e : node->GetChildren()) TranslateNode(e);
            return;
    }

    Fail("Unsupported node %s", node->ToString().c_str());
    KAI_NOT_IMPLEMENTED();
}

void HlslTranslator::TranslateBlock(const AstNodePtr &node) {
    for (const auto st : node->GetChildren()) TranslateNode(st);
}

void HlslTranslator::TranslateFunction(const AstNodePtr &node) {
    // child 0: ident
    // child 1: args
    // child 2: block
    AstNode::ChildrenType const &ch = node->GetChildren();

    // write the body
    PushNew();
    for (auto b : ch[2]->GetChildren()) TranslateNode(b);

    // add the args
    auto cont = Pop();
    for (auto a : ch[1]->GetChildren()) cont->AddArg(Label(a->GetTokenText()));

    // write the name and store
    Append(cont);
    AppendNew(Label(ch[0]->Text()));
    AppendOp(Operation::Store);
}

void HlslTranslator::TranslateCall(AstNodePtr node) {
    typename AstNode::ChildrenType const &children = node->GetChildren();
    for (auto a : children[1]->GetChildren()) TranslateNode(a);

    TranslateNode(children[0]);
    /*    if (children.size() > 2 && children[2]->GetToken().type ==
       TokenEnum::Replace) AppendNew(Operation(Operation::Replace)); else*/
    AppendNew(Operation(Operation::Suspend));
}

void HlslTranslator::TranslateIf(AstNodePtr node) {
    typename AstNode::ChildrenType const &ch = node->GetChildren();
    bool hasElse = ch.size() > 2;
    TranslateNode(ch[0]);
    if (hasElse) TranslateNode(ch[2]);

    TranslateNode(ch[1]);
    AppendOp(hasElse ? Operation::IfThenSuspendElseSuspend
                     : Operation::IfThenSuspend);
}

void HlslTranslator::TranslateFor(AstNodePtr node) {
    AppendOp(Operation::None);
}

void HlslTranslator::TranslateWhile(AstNodePtr node) {
    AppendOp(Operation::None);
}

KAI_END

#include "KAI/Language/Pi/PiTranslator.h"

#include <boost/lexical_cast.hpp>

using namespace boost;
using namespace std;

KAI_BEGIN

void PiTranslator::TranslateNode(AstNodePtr node) {
    if (!node) {
        Fail("Empty input");
        return;
    }

    switch (node->GetType()) {
        case PiAstNodeEnumType::Array: {
            // Although we are getting an array,
            // pretend at first we are getting a Continuation
            // then use it's Code as the data for the array.
            PushNew();
            for (auto const &ch : node->GetChildren()) TranslateNode(ch);
            Pointer<Continuation> c = Pop();
            Append(c->GetCode());
            break;
        }

        case PiAstNodeEnumType::Continuation: {
            PushNew();
            for (auto const &ch : node->GetChildren()) TranslateNode(ch);
            Append(Pop());
            break;
        }

        default: {
            AppendTokenised(node->GetToken());
            break;
        }
    }
}

void PiTranslator::AppendTokenised(const TokenNode &tok) {
    switch (tok.type) {
        case PiTokenEnumType::String:
            AppendNew(String(tok.Text()));
            break;

        case PiTokenEnumType::QuotedIdent: {
            auto label = Label(tok.Text().c_str());
            label.SetQuoted(true);
            AppendNew(label);
            break;
        }

        case PiTokenEnumType::True:
            AppendNew(true);
            break;

        case PiTokenEnumType::Assert:
            AppendOp(Operation::Assert);
            break;

        case PiTokenEnumType::False:
            AppendNew(false);
            break;

        case PiTokenEnumType::Bool:
            AppendNew(boost::lexical_cast<bool>(tok.Text()));
            break;

        case PiTokenEnumType::GetType:
            AppendOp(Operation::TypeOf);
            break;

        case PiTokenEnumType::Store:
            AppendOp(Operation::Store);
            break;

        case PiTokenEnumType::Lookup:
            AppendOp(Operation::Retreive);
            break;

        case PiTokenEnumType::Int:
            AppendNew(boost::lexical_cast<int>(tok.Text()));
            break;

        case PiTokenEnumType::Replace:
            AppendOp(Operation::Replace);
            break;

        case PiTokenEnumType::Suspend:
            AppendOp(Operation::Suspend);
            break;

        case PiTokenEnumType::Resume:
            AppendOp(Operation::Resume);
            break;

        case PiTokenEnumType::Drop:
            AppendOp(Operation::Drop);
            break;

        case PiTokenEnumType::Dup:
            AppendOp(Operation::Dup);
            break;

        case PiTokenEnumType::Assign:
            AppendOp(Operation::Assign);
            break;

        case PiTokenEnumType::Swap:
            AppendOp(Operation::Swap);
            break;

        case PiTokenEnumType::Plus:
            AppendOp(Operation::Plus);
            break;

        case PiTokenEnumType::Minus:
            AppendOp(Operation::Minus);
            break;

        case PiTokenEnumType::Mul:
            AppendOp(Operation::Multiply);
            break;

        case PiTokenEnumType::Divide:
            AppendOp(Operation::Divide);
            break;

        case PiTokenEnumType::Equiv:
            AppendOp(Operation::Equiv);
            break;

        case PiTokenEnumType::Greater:
            AppendOp(Operation::Greater);
            break;

        case PiTokenEnumType::GreaterEquiv:
            AppendOp(Operation::GreaterOrEquiv);
            break;

        case PiTokenEnumType::LessEquiv:
            AppendOp(Operation::LessOrEquiv);
            break;

        case PiTokenEnumType::Rot:
            AppendOp(Operation::Rot);
            break;

        case PiTokenEnumType::Over:
            AppendOp(Operation::Over);
            break;

        case PiTokenEnumType::PickN:
            AppendOp(Operation::Pick);
            break;

        case PiTokenEnumType::Clear:
            AppendOp(Operation::Clear);
            break;

        case PiTokenEnumType::GarbageCollect:
            AppendOp(Operation::GarbageCollect);
            break;

        case PiTokenEnumType::Ident:
            AppendNew(Label(tok.Text()));
            break;

        case PiTokenEnumType::Pathname:
            AppendNew(Pathname(tok.Text()));
            break;

        case PiTokenEnumType::ToRho:
            AppendOp(Operation::ToRho);
            break;
        case PiTokenEnumType::None:
            break;

        case PiTokenEnumType::Size:
            AppendOp(Operation::Size);
            break;

        case PiTokenEnumType::ToArray:
            AppendOp(Operation::ToArray);
            break;

        case PiTokenEnumType::Depth:
            AppendOp(Operation::Depth);
            break;

        case PiTokenEnumType::Not:
            AppendOp(Operation::LogicalNot);
            break;

        case PiTokenEnumType::And:
            AppendOp(Operation::LogicalAnd);
            break;

        case PiTokenEnumType::New:
            AppendOp(Operation::New);
            break;

        case PiTokenEnumType::Or:
            AppendOp(Operation::LogicalOr);
            break;

        case PiTokenEnumType::Xor:
            AppendOp(Operation::LogicalXor);
            break;

        case PiTokenEnumType::If:
            AppendOp(Operation::If);
            break;

        case PiTokenEnumType::IfElse:
            AppendOp(Operation::IfElse);
            break;

        case PiTokenEnumType::Exists:
            AppendOp(Operation::Exists);
            break;

        case PiTokenEnumType::Expand:
            AppendOp(Operation::Expand);
            break;

        case PiTokenEnumType::Tab:
            return;

        case PiTokenEnumType::PrintFolder:
            AppendOp(Operation::GetScope);
            return;

        case PiTokenEnumType::ChangeFolder:
            AppendOp(Operation::ChangeScope);
            return;

        case PiTokenEnumType::NotEquiv:
            AppendOp(Operation::NotEquiv);
            return;

        case PiTokenEnumType::Freeze:
            AppendOp(Operation::Freeze);
            return;

        case PiTokenEnumType::Thaw:
            AppendOp(Operation::Thaw);
            return;

        case PiTokenEnumType::This:
            AppendOp(Operation::This);
            return;

        case PiTokenEnumType::Self:
            AppendOp(Operation::Self);
            return;

        case PiTokenEnumType::Contents:
            AppendOp(Operation::Contents);
            return;

        case PiTokenEnumType::GetContents:
            AppendOp(Operation::GetChildren);
            return;

        case PiTokenEnumType::DropN:
            AppendOp(Operation::DropN);
            return;

        case PiTokenEnumType::ToList:
            AppendOp(Operation::ToList);
            return;

        // Note: "call" was added here but removed due to conflict

        default:
            KAI_TRACE_1(tok.type)
                << ": PiTranslator: token not implemented: " << tok.ToString();
            break;
    }
}

KAI_END

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
            // In Pi language, continuations created with {} stay on the stack until 
            // explicitly executed with & or !
            
            // Create a new continuation for the {} block
            Pointer<Continuation> cont = reg_->New<Continuation>();
            Pointer<Array> code = reg_->New<Array>();
            
            // First, add an operation marker to indicate the start of a continuation block
            // This helps the executor properly handle Pi continuations
            AppendDirectOperation(code, Operation::ContinuationBegin);
            
            // For empty continuations '{}' in Pi language
            if (node->GetChildren().empty()) {
                // For empty continuations, just add the end marker and we're done
                AppendDirectOperation(code, Operation::ContinuationEnd);
                cont->SetCode(code);
                
                // The continuation markers are sufficient, no need for extra properties
                
                Append(cont);
                break;
            }
            
            // For non-empty continuations, translate all child nodes
            PushNew();
            for (auto const &ch : node->GetChildren()) {
                TranslateNode(ch);
            }
            
            // Get the inner continuation with all operations
            Pointer<Continuation> innerCont = Pop();
            
            // Copy all operations to our new continuation
            if (innerCont->GetCode().Exists()) {
                for (int i = 0; i < innerCont->GetCode()->Size(); ++i) {
                    code->Append(innerCont->GetCode()->At(i));
                }
            }
            
            // Add the end marker for this continuation block
            AppendDirectOperation(code, Operation::ContinuationEnd);
            
            // Set the code array and append the continuation
            cont->SetCode(code);
            
            // The continuation markers are sufficient, no need for extra properties
            
            Append(cont);
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
            // Fix for assert operation in Pi language
            // Assert operation checks if top value on the stack is true
            // Making sure this is handled correctly by using direct operation call
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
            // In Pi, 'Store' (# operator) stores a value with a label
            // We need to make sure it preserves type when storing
            // Store operation must ensure exact type is preserved
            AppendOp(Operation::Store);
            break;

        case PiTokenEnumType::Lookup:
            // In Pi, 'Lookup' (@ operator) retrieves a value by label
            // We need to make sure it maintains proper type information during retrieval
            // and correctly handles error cases when variables don't exist
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
            // Make sure both 'div' and '/' tokens are handled as division operations
            // This is critical for Pi language division operations to work correctly
            AppendOp(Operation::Divide);
            break;

        case PiTokenEnumType::Modulo:
            AppendOp(Operation::Modulo);
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
            // Fix for variable identifier handling in Pi language
            // Create a Label with the variable name
            AppendNew(Label(tok.Text().c_str()));
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
            // Make sure Size operation is correctly identified and appended
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

// Helper method to append an operation directly to a code array
void PiTranslator::AppendDirectOperation(Pointer<Array> code, Operation::Type opType) {
    Object op = reg_->New<Operation>(opType);
    code->Append(op);
}

KAI_END

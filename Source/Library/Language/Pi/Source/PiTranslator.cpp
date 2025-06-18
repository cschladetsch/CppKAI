#include "KAI/Language/Pi/PiTranslator.h"

#include <boost/lexical_cast.hpp>
#include <iostream>

#include "KAI/Core/BuiltinTypes.h"
#include "KAI/Core/BuiltinTypes/List.h"
#include "KAI/Core/BuiltinTypes/Map.h"
#include "KAI/Core/BuiltinTypes/String.h"

using namespace boost;
using namespace std;

KAI_BEGIN

PiTranslator::~PiTranslator() = default;

// Implementation of the Translate method for Pi
Pointer<Continuation> PiTranslator::Translate(const char *text, Structure st) {
    // Set flag to indicate we're translating the root
    isTranslatingRoot = true;

    // Call the parent implementation first
    Pointer<Continuation> cont = Parent::Translate(text, st);

    // Reset flag
    isTranslatingRoot = false;

    // No special flag needed - we extract primitive types during execution
    if (cont.Exists()) {
    }

    return cont;
}

void PiTranslator::TranslateNode(AstNodePtr node) {
    if (!node) {
        Fail("Empty input");
        return;
    }

    switch (node->GetType()) {
        case PiAstNodeEnumType::Array: {
            // Debug print for array node
            if (node->GetToken().type != PiTokenEnumType::None) {
                std::cout << "Array node with token: "
                          << node->GetToken().Text() << std::endl;
            } else {
                std::cout << "Array node with children: "
                          << node->GetChildren().size() << std::endl;
            }

            // For empty array, create it directly
            if (node->GetChildren().empty()) {
                Object emptyArray = reg_->New<Array>();
                Append(emptyArray);
                break;
            }

            // For non-empty arrays, we need to handle two approaches:
            // 1. Create array with elements directly for simple cases
            // 2. Use proper ToArray operation for more complex cases

            // First check if all children are simple literals (int, string,
            // bool)
            bool allSimpleLiterals = true;
            for (auto const &ch : node->GetChildren()) {
                if (ch->GetToken().type != PiTokenEnumType::Int &&
                    ch->GetToken().type != PiTokenEnumType::String &&
                    ch->GetToken().type != PiTokenEnumType::Bool) {
                    allSimpleLiterals = false;
                    break;
                }
            }

            // If all simple literals, create array directly
            if (allSimpleLiterals) {
                Object arrayObj = reg_->New<Array>();
                Array &array = Deref<Array>(arrayObj);

                // Process all array elements and add them directly to the array
                for (auto const &ch : node->GetChildren()) {
                    // For integers, handle them directly
                    if (ch->GetToken().type == PiTokenEnumType::Int) {
                        int value =
                            boost::lexical_cast<int>(ch->GetToken().Text());
                        array.Append(reg_->New<int>(value));
                    }
                    // Handle other literal types as needed
                    else if (ch->GetToken().type == PiTokenEnumType::String) {
                        String value = ch->GetToken().Text();
                        array.Append(reg_->New<String>(value));
                    } else if (ch->GetToken().type == PiTokenEnumType::Bool) {
                        bool value =
                            boost::lexical_cast<bool>(ch->GetToken().Text());
                        array.Append(reg_->New<bool>(value));
                    }
                }

                // Append the completed array object
                Append(arrayObj);
            }
            // For more complex cases, use the normal array creation approach
            else {
                // First add the array size
                AppendNew(static_cast<int>(node->GetChildren().size()));

                // Add all elements in reverse order
                for (auto it = node->GetChildren().rbegin();
                     it != node->GetChildren().rend(); ++it) {
                    TranslateNode(*it);
                }

                // Finally, add the ToArray operation
                AppendOp(Operation::ToArray);
            }
            break;
        }

        case PiAstNodeEnumType::Continuation: {
            // Check if this is a root-level continuation from the parser
            // (not an explicit {} block in the code)
            if (isTranslatingRoot) {
                // This is the root continuation created by the parser
                // Just translate its children directly without wrapping
                isTranslatingRoot = false;  // Reset flag after processing root
                for (auto const &ch : node->GetChildren()) {
                    TranslateNode(ch);
                }
                break;
            }

            // This is an explicit {} block in Pi language
            // These continuations stay on the stack until explicitly executed
            // with & or !

            // Create a new continuation for the {} block
            Pointer<Continuation> cont = reg_->New<Continuation>();
            Pointer<Array> code = reg_->New<Array>();

            // First, add an operation marker to indicate the start of a
            // continuation block This helps the executor properly handle Pi
            // continuations
            AppendDirectOperation(code, Operation::ContinuationBegin);

            // For empty continuations '{}' in Pi language
            if (node->GetChildren().empty()) {
                // For empty continuations, just add the end marker and we're
                // done
                AppendDirectOperation(code, Operation::ContinuationEnd);
                cont->SetCode(code);

                // The continuation markers are sufficient, no need for extra
                // properties

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

            // The continuation markers are sufficient, no need for extra
            // properties

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
            // Use Pathname for quoted identifiers
            AppendNew(Pathname(tok.Text()));
            break;
        }

        case PiTokenEnumType::True:
            AppendNew(true);
            break;

        case PiTokenEnumType::Assert:
            // Fix for assert operation in Pi language
            // Assert operation checks if top value on the stack is true
            // Making sure this is handled correctly by using direct operation
            // call
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
            // We need to make sure it maintains proper type information during
            // retrieval and correctly handles error cases when variables don't
            // exist
            AppendOp(Operation::Retreive);
            break;

        case PiTokenEnumType::Int:
            AppendNew(boost::lexical_cast<int>(tok.Text()));
            break;

        case PiTokenEnumType::Float:
            AppendNew(boost::lexical_cast<float>(tok.Text()));
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

        case PiTokenEnumType::Dup2:
            AppendOp(Operation::Dup2);
            break;

        case PiTokenEnumType::Drop2:
            AppendOp(Operation::Drop2);
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
            // Make sure both 'div' and '/' tokens are handled as division
            // operations This is critical for Pi language division operations
            // to work correctly
            AppendOp(Operation::Divide);
            break;

        case PiTokenEnumType::Modulo:
            AppendOp(Operation::Modulo);
            break;

        case PiTokenEnumType::Equiv:
            AppendOp(Operation::Equiv);
            break;

        case PiTokenEnumType::Less:
            AppendOp(Operation::Less);
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

        case PiTokenEnumType::Roll:
            AppendOp(Operation::Roll);
            break;

        case PiTokenEnumType::Min:
            AppendOp(Operation::Min);
            break;

        case PiTokenEnumType::Max:
            AppendOp(Operation::Max);
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
            // Unquoted identifiers need to be looked up
            AppendNew(Label(tok.Text()));
            AppendOp(Operation::Lookup);
            break;

        case PiTokenEnumType::Pathname:
            // Pathname tokens include the quote if present
            KAI_TRACE() << "Pathname token text: '" << tok.Text() << "'";
            AppendNew(Pathname(tok.Text()));
            break;

        case PiTokenEnumType::ToRho:
            AppendOp(Operation::ToRho);
            break;
        case PiTokenEnumType::None:
            break;

        case PiTokenEnumType::Size: {
            // Debug print for size operation
            std::cout << "Size operation detected, stack size: " << stack.size()
                      << std::endl;

            // Special case for "[] size" - directly push 0
            if (stack.size() > 0) {
                auto cont = stack.back();
                if (cont->GetCode().Exists()) {
                    if (cont->GetCode()->Size() == 1) {
                        auto firstItem = cont->GetCode()->At(0);
                        // Check if it's an empty array
                        if (firstItem.IsType<Array>() &&
                            Deref<Array>(firstItem).Size() == 0) {
                            std::cout
                                << "Empty array detected, pushing 0 directly"
                                << std::endl;
                            // Replace the array with integer 0
                            Pointer<Array> newCode = reg_->New<Array>();
                            newCode->Append(reg_->New<int>(0));
                            cont->SetCode(newCode);
                            return;
                        }
                    }

                    // Check if the last item is an array (non-empty)
                    if (cont->GetCode()->Size() > 0) {
                        auto lastItem =
                            cont->GetCode()->At(cont->GetCode()->Size() - 1);

                        if (lastItem.IsType<Array>()) {
                            // Get the array directly
                            Array &array = Deref<Array>(lastItem);
                            std::cout << "Array found, size: " << array.Size()
                                      << std::endl;

                            // Create a new code array without the array
                            Pointer<Array> newCode = reg_->New<Array>();
                            // Copy all but the last item (the array)
                            for (int i = 0; i < cont->GetCode()->Size() - 1;
                                 i++) {
                                newCode->Append(cont->GetCode()->At(i));
                            }
                            // Add the size instead
                            newCode->Append(reg_->New<int>(array.Size()));
                            // Replace the code
                            cont->SetCode(newCode);
                            return;
                        }
                    }
                }
            }

            // For all other cases, use the Size operation
            std::cout << "Using regular Size operation" << std::endl;
            AppendOp(Operation::Size);
            break;
        }

        case PiTokenEnumType::ToArray:
            // Debug print
            std::cout << "ToArray operation detected" << std::endl;

            // Our improved Executor.ToArray implementation properly handles
            // empty arrays and array creation from stack elements
            AppendOp(Operation::ToArray);
            break;

        case PiTokenEnumType::ToMap:
            AppendOp(Operation::ToMap);
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

        case PiTokenEnumType::BitAnd:
            AppendOp(Operation::BitwiseAnd);
            break;

        case PiTokenEnumType::BitOr:
            AppendOp(Operation::BitwiseOr);
            break;

        case PiTokenEnumType::BitXor:
            AppendOp(Operation::BitwiseXor);
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

        case PiTokenEnumType::While:
            AppendOp(Operation::WhileLoop);
            return;

        case PiTokenEnumType::For:
            AppendOp(Operation::ForLoop);
            return;

        case PiTokenEnumType::Print:
            AppendOp(Operation::Print);
            return;

        case PiTokenEnumType::ShellCommand: {
            // Push the shell command string and execute it
            auto commandText = tok.Text();
            Append(New<String>(commandText));
            AppendOp(Operation::ShellCommand);
            return;
        }

        case PiTokenEnumType::ToStr:
            AppendOp(Operation::ToStringOp);
            return;

            // Note: "call" was added here but removed due to conflict

        default:
            KAI_TRACE_1(tok.type)
                << ": PiTranslator: token not implemented: " << tok.ToString();
            break;
    }
}

// Helper method to append an operation directly to a code array
void PiTranslator::AppendDirectOperation(Pointer<Array> code,
                                         Operation::Type opType) {
    Object op = reg_->New<Operation>(opType);
    code->Append(op);
}

KAI_END

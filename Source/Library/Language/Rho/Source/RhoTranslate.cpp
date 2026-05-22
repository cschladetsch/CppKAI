#include <KAI/Core/BuiltinTypes.h>
#include <KAI/Executor/Operation.h>
#include <KAI/Language/Rho/RhoTranslator.h>

// Use C++20/23 headers
#include <concepts>
#include <format>
#include <iostream>
#include <optional>
#include <ranges>
#include <set>
#include <string_view>

KAI_BEGIN

/// Implementation of the TranslateNode method that processes AST nodes
void RhoTranslator::TranslateNode(AstNodePtr node) {
    if (!node) {
        KAI_TRACE_ERROR() << "NULL node in TranslateNode";
        return;
    }

    KAI_TRACE() << "Processing node: " << node->ToString();

    // Only handle the minimal necessary cases for the basic functionality
    switch (node->GetType()) {
        case AstNodeEnum::None:
            KAI_TRACE() << "Empty node encountered, skipping";
            break;

        case AstNodeEnum::Program:
            KAI_TRACE() << "Processing Program node";
            for (const auto& child : node->GetChildren()) {
                TranslateNode(child);
            }
            break;

        case AstNodeEnum::TokenType:
            KAI_TRACE() << "Processing token node: "
                        << RhoTokenEnumType::ToString(node->GetToken().type);
            TranslateToken(node);
            break;

        case AstNodeEnum::DoWhile:
            KAI_TRACE() << "Processing DoWhile node";
            TranslateDoWhile(node);
            break;

        case AstNodeEnum::While:
            KAI_TRACE() << "Processing While node";
            TranslateWhile(node);
            break;

        case AstNodeEnum::For:
            KAI_TRACE() << "Processing For node";
            TranslateFor(node);
            break;

        case AstNodeEnum::ForEach:
            KAI_TRACE() << "Processing ForEach node";
            TranslateForEach(node);
            break;

        case AstNodeEnum::Conditional:
            KAI_TRACE() << "Processing If node";
            TranslateIf(node);
            break;

        case AstNodeEnum::List:
            KAI_TRACE() << "Processing List node";
            TranslateList(node);
            break;

        case AstNodeEnum::IndexOp:
            KAI_TRACE() << "Processing IndexOp node";
            TranslateIndex(node);
            break;

        case AstNodeEnum::Map:
            KAI_TRACE() << "Processing Map node";
            TranslateMap(node);
            break;

        case AstNodeEnum::ToPiLang:
            KAI_TRACE() << "Processing ToPiLang node";
            TranslatePiBlock(node);
            break;

        case AstNodeEnum::Function:
            KAI_TRACE() << "Processing Function node";
            TranslateFunction(node);
            // Don't process children - they're handled by TranslateFunction
            return;

        case AstNodeEnum::Call:
            KAI_TRACE() << "Processing Call node";
            TranslateCall(node);
            break;

        case AstNodeEnum::Block:
            KAI_TRACE() << "Processing Block node";
            TranslateBlock(node);
            break;

        case AstNodeEnum::Assignment: {
            KAI_TRACE() << "Processing Assignment node";

            auto valueNode = node->GetChild(0);
            auto targetNode = node->GetChild(1);

            // Translate the value (right-hand side)
            TranslateNode(valueNode);

            // For function assignments, TranslateFunction already handles
            // storage so we don't need to add another Store operation
            if (valueNode->GetType() == AstNodeEnum::Function) {
                KAI_TRACE() << "Assignment of function - storage handled by "
                               "TranslateFunction";
                break;
            }

            // Special-case member assignment: obj.member = value
            if (targetNode->GetType() == AstNodeEnum::GetMember) {
                auto objectNode = targetNode->GetChild(0);
                auto memberNode = targetNode->GetChild(1);

                TranslateNode(objectNode);
                if (memberNode->GetType() == AstNodeEnum::TokenType &&
                    memberNode->GetToken().type == TokenEnum::Label) {
                    Append(
                        reg_->New<String>(String(memberNode->GetTokenText())));
                } else {
                    TranslateNode(memberNode);
                }

                // Stack: value, object, key -> rotate to object, key, value
                AppendDirectOperation(Operation::Rot);
                AppendDirectOperation(Operation::SetChild);
                break;
            }

            // For the target (left-hand side), create a quoted pathname
            // instead of translating it normally (which would add Retrieve)
            if (targetNode->GetType() == AstNodeEnum::Ident ||
                (targetNode->GetType() == AstNodeEnum::TokenType &&
                 targetNode->GetToken().type == TokenEnum::Label)) {
                // Create quoted pathname for assignment target
                String targetName = targetNode->GetTokenText();
                String quotedPath = "'" + targetName;
                auto pathObj = reg_->New<Pathname>(Pathname(quotedPath));
                Append(pathObj);
            } else {
                // For complex targets (e.g., array indexing), translate
                // normally
                TranslateNode(targetNode);
            }

            AppendDirectOperation(Operation::Store);
            break;
        }

        case AstNodeEnum::Ident:
            KAI_TRACE() << "Processing Ident node";
            TranslateToken(node);
            break;

        case AstNodeEnum::GetMember:
            KAI_TRACE() << "Processing GetMember node";
            // GetMember nodes have: object and member name
            if (node->GetChildren().size() >= 2) {
                TranslateNode(node->GetChild(0));  // Object
                auto memberNode = node->GetChild(1);
                if (memberNode->GetType() == AstNodeEnum::TokenType &&
                    memberNode->GetToken().type == TokenEnum::Label) {
                    Append(
                        reg_->New<String>(String(memberNode->GetTokenText())));
                } else {
                    TranslateNode(memberNode);
                }
                AppendDirectOperation(Operation::GetChild);
            }
            break;

        case AstNodeEnum::ArgList:
            KAI_TRACE() << "Processing ArgList node";
            // ArgList is just a container for arguments, process all children
            for (const auto& child : node->GetChildren()) {
                TranslateNode(child);
            }
            break;

        default:
            // Log warning about unhandled node type but continue
            KAI_TRACE() << "Node type not fully implemented: "
                        << RhoAstNodeEnumType::ToString(node->GetType());

            // Recursively process child nodes so we don't completely halt
            for (const auto& child : node->GetChildren()) {
                TranslateNode(child);
            }
            break;
    }
}

Pointer<Continuation> RhoTranslator::Translate(const char* text, Structure st) {
    return Parent::Translate(text, st);
}

KAI_END

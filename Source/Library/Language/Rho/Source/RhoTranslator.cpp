#include <KAI/Core/BuiltinTypes.h>
#include <KAI/Executor/Operation.h>
#include <KAI/Language/Rho/RhoTranslator.h>

#include <boost/lexical_cast.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <iostream>  // For std::cout and std::endl

using namespace std;

KAI_BEGIN

void RhoTranslator::TranslateToken(AstNodePtr node) {
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

        case TokenEnum::Assert:
            TranslateNode(node->GetChild(0));
            AppendOp(Operation::Assert);
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

        case TokenEnum::Lookup:
            AppendOp(Operation::Lookup);
            return;

        case TokenEnum::Self:
            AppendOp(Operation::This);
            return;

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

        case TokenEnum::Pathname:
            Append(New<Pathname>(Pathname(node->Text())));
            return;

        case TokenEnum::ToPi:
            AppendOp(Operation::ToPi);
            return;

        case TokenEnum::Yield:
            // for (auto ch : node->Children)
            //     Translate(ch);
            // AppendNewOp(Operation::PushContext);
            KAI_NOT_IMPLEMENTED();
            return;

        case TokenEnum::Return:
            for (auto ch : node->GetChildren()) TranslateNode(ch);
            AppendOp(Operation::Return);
            return;
    }

    Fail("Unsupported node %s", node->ToString().c_str());
    KAI_TRACE_ERROR() << "Error: " << Error;
    KAI_NOT_IMPLEMENTED();
}

void RhoTranslator::TranslateBinaryOp(AstNodePtr node, Operation::Type op) {
    TranslateNode(node->GetChild(0));
    TranslateNode(node->GetChild(1));

    AppendNew(Operation(op));
}

// void RhoTranslator::TranslatePathname(AstNodePtr node)
//{
//     Pathname::Elements elements;
//     typedef Pathname::Element El;
//
//     for (auto ch : node->GetChildren())
//     {
//         switch (ch->GetToken().type)
//         {
//         case RhoTokenEnumType::Quote:
//             elements.push_back(El::Quote);
//             break;
//         case RhoTokenEnumType::Sep:
//             elements.push_back(El::Separator);
//             break;
//         case RhoTokenEnumType::Ident:
//             elements.push_back(Label(ch->GetTokenText()));
//             break;
//         }
//     }
//
//     AppendNew(Pathname(move(elements)));
// }

void RhoTranslator::TranslateNode(AstNodePtr node) {
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

        case AstEnum::While:
            TranslateWhile(node);
            return;

        case AstEnum::DoWhile:
            TranslateDoWhile(node);
            return;

        case AstEnum::Block:
            PushNew();
            for (auto st : node->GetChildren()) TranslateNode(st);
            Append(Pop());
            return;

        case AstEnum::List:
            KAI_NOT_IMPLEMENTED();
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

void RhoTranslator::TranslateBlock(AstNodePtr node) {
    for (auto st : node->GetChildren()) TranslateNode(st);
}

void RhoTranslator::TranslateFunction(AstNodePtr node) {
    // child 0: ident
    // child 1: args
    // child 2: block
    AstNode::ChildrenType const &ch = node->GetChildren();

    // write the body
    PushNew();
    for (auto b : ch[2]->GetChildren()) {
        // Special handling for return statements
        if (b->GetType() == AstEnum::TokenType &&
            b->GetToken().type == TokenEnum::Return) {
            // Process the return value if it exists
            if (b->GetChildren().size() > 0) {
                TranslateNode(b->GetChild(0));
            }

            // Add the Return operation
            AppendOp(Operation::Return);
        } else {
            TranslateNode(b);
        }
    }

    // add the args
    auto cont = Pop();
    for (auto a : ch[1]->GetChildren()) cont->AddArg(Label(a->GetTokenText()));

    // write the name and store
    Append(cont);
    AppendNew(Label(ch[0]->Text()));
    AppendOp(Operation::Store);
}

void RhoTranslator::TranslateCall(AstNodePtr node) {
    typename AstNode::ChildrenType const &children = node->GetChildren();
    for (auto a : children[1]->GetChildren()) TranslateNode(a);

    TranslateNode(children[0]);
    if (children.size() > 2 &&
        children[2]->GetToken().type == TokenEnum::Replace)
        AppendNew(Operation(Operation::Replace));
    else
        AppendNew(Operation(Operation::Suspend));
}

void RhoTranslator::TranslateIf(AstNodePtr node) {
    typename AstNode::ChildrenType const &ch = node->GetChildren();
    bool hasElse = ch.size() > 2;
    TranslateNode(ch[0]);
    if (hasElse) TranslateNode(ch[2]);

    TranslateNode(ch[1]);
    AppendOp(hasElse ? Operation::IfThenSuspendElseSuspend
                     : Operation::IfThenSuspend);
}

void RhoTranslator::TranslateWhile(AstNodePtr node) {
    // Enhanced implementation for while loops
    try {
        KAI_TRACE() << "---------------------------------------------";
        KAI_TRACE() << "Starting translation of while loop with enhanced debugging";

        // Verify we have enough children
        if (node->GetChildren().size() < 1) {
            KAI_TRACE_ERROR() << "Not enough children in While node";
            Fail("While node must have at least a condition child");
            return;
        }

        // Get condition and body nodes
        AstNodePtr condition = node->GetChild(0);
        KAI_TRACE() << "Condition node type: " << RhoAstNodeEnumType::ToString(condition->GetType());

        AstNodePtr body = nullptr;
        if (node->GetChildren().size() > 1) {
            body = node->GetChild(1);
            KAI_TRACE() << "Body node type: " << RhoAstNodeEnumType::ToString(body->GetType());
        } else {
            // Create an empty body block node
            body = std::make_shared<RhoAstNode>(RhoAstNodeEnumType::Block);
            KAI_TRACE() << "Created empty body block node";
        }

        // 1. Create the condition continuation
        KAI_TRACE() << "Creating condition continuation";
        Pointer<Continuation> condCont = _reg->New<Continuation>();
        if (!condCont.Exists()) {
            KAI_TRACE_ERROR() << "Failed to create condition continuation";
            Fail("Failed to create condition continuation");
            return;
        }

        // Set its code array
        condCont->SetCode(_reg->New<Array>());
        if (!condCont->GetCode().Exists()) {
            KAI_TRACE_ERROR() << "Failed to create condition code array";
            Fail("Failed to create condition code array");
            return;
        }

        // 2. Create the body continuation
        KAI_TRACE() << "Creating body continuation";
        Pointer<Continuation> bodyCont = _reg->New<Continuation>();
        if (!bodyCont.Exists()) {
            KAI_TRACE_ERROR() << "Failed to create body continuation";
            Fail("Failed to create body continuation");
            return;
        }

        // Set its code array
        bodyCont->SetCode(_reg->New<Array>());
        if (!bodyCont->GetCode().Exists()) {
            KAI_TRACE_ERROR() << "Failed to create body code array";
            Fail("Failed to create body code array");
            return;
        }

        // 3. Translate the condition node into the condition continuation
        KAI_TRACE() << "Translating condition node into condition continuation";
        stack.push_back(condCont);
        TranslateNode(condition);
        stack.pop_back();

        // 4. Translate the body node into the body continuation
        KAI_TRACE() << "Translating body node into body continuation";
        stack.push_back(bodyCont);
        TranslateNode(body);
        stack.pop_back();

        // 5. Verify continuations have proper code arrays
        if (!condCont->GetCode().Exists()) {
            KAI_TRACE_ERROR() << "Condition continuation has invalid code array after translation";
            Fail("Condition continuation has invalid code array after translation");
            return;
        }

        if (!bodyCont->GetCode().Exists()) {
            KAI_TRACE_ERROR() << "Body continuation has invalid code array after translation";
            Fail("Body continuation has invalid code array after translation");
            return;
        }

        // 6. Add condition and body continuations to the current code array
        // Push condition continuation then body continuation (order matters for while)
        KAI_TRACE() << "Adding condition continuation to result";
        Append(condCont);  // Append takes Object by const reference

        KAI_TRACE() << "Adding body continuation to result";
        Append(bodyCont);  // Append takes Object by const reference

        // 7. Add WhileLoop operation
        KAI_TRACE() << "Adding WhileLoop operation";
        AppendOp(Operation::WhileLoop);

        KAI_TRACE() << "Successfully completed while loop translation";
        KAI_TRACE() << "---------------------------------------------";
    } catch (std::exception &e) {
        KAI_TRACE_ERROR() << "Exception in TranslateWhile: " << e.what();
        Fail(std::string("Exception in TranslateWhile: ") + e.what());
    } catch (...) {
        KAI_TRACE_ERROR() << "Unknown exception in TranslateWhile";
        Fail("Unknown exception in TranslateWhile");
    }
}

void RhoTranslator::TranslateDoWhile(AstNodePtr node) {
    // Implementation for do-while loops
    try {
        KAI_TRACE() << "---------------------------------------------";
        KAI_TRACE() << "Starting translation of do-while loop with enhanced debugging";

        // Verify we have enough children
        int childCount = node->GetChildren().size();
        KAI_TRACE() << "DoWhile node has " << childCount << " children";

        if (childCount < 2) {
            KAI_TRACE_ERROR() << "Not enough children in DoWhile node";
            Fail("DoWhile node must have both body and condition children");
            return;
        }

        // Get body and condition nodes (order is different from while loop)
        AstNodePtr body = node->GetChild(0);
        AstNodePtr condition = node->GetChild(1);

        KAI_TRACE() << "Body node type: " << RhoAstNodeEnumType::ToString(body->GetType());
        KAI_TRACE() << "Condition node type: " << RhoAstNodeEnumType::ToString(condition->GetType());

        // Instead of creating continuations separately and translating nodes into them,
        // we'll create the continuations directly during translation

        // 1. Create and translate the body continuation
        KAI_TRACE() << "Creating and translating body continuation";
        PushNew(); // Start a new code section for body
        TranslateNode(body);
        auto bodyCont = Pop(); // Get the body continuation
        KAI_TRACE() << "Body continuation created";

        // 2. Create and translate the condition continuation
        KAI_TRACE() << "Creating and translating condition continuation";
        PushNew(); // Start a new code section for condition
        TranslateNode(condition);
        auto condCont = Pop(); // Get the condition continuation
        KAI_TRACE() << "Condition continuation created";

        // 3. Add body and condition continuations to the current code array
        // Push body continuation then condition continuation (order matters for do-while)
        KAI_TRACE() << "Adding continuations to the stack";
        Append(bodyCont);
        Append(condCont);

        // 4. Add DoLoop operation
        KAI_TRACE() << "Adding DoLoop operation";
        AppendOp(Operation::DoLoop);

        KAI_TRACE() << "Successfully completed do-while translation";
        KAI_TRACE() << "---------------------------------------------";
    } catch (std::exception &e) {
        KAI_TRACE_ERROR() << "Exception in TranslateDoWhile: " << e.what();
        Fail(std::string("Exception in TranslateDoWhile: ") + e.what());
    } catch (...) {
        KAI_TRACE_ERROR() << "Unknown exception in TranslateDoWhile";
        Fail("Unknown exception in TranslateDoWhile");
    }
}

KAI_END
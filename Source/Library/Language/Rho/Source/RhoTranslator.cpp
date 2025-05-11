#include <KAI/Core/BuiltinTypes.h>
#include <KAI/Executor/Operation.h>
#include <KAI/Language/Rho/RhoTranslator.h>

#include <boost/lexical_cast.hpp>
#include <boost/range/adaptor/reversed.hpp>

using namespace std;

KAI_BEGIN

void RhoTranslator::TranslateToken(AstNodePtr node)
{
    KAI_TRACE() << "TranslateToken: " << RhoTokenEnumType::ToString(node->GetToken().type);
    KAI_TRACE() << "TranslateToken with text: " << node->Text();

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
            KAI_TRACE() << "Translating Plus operation";
            TranslateBinaryOp(node, Operation::Plus);
            return;

        case TokenEnum::Mul:
            TranslateBinaryOp(node, Operation::Multiply);
            return;

        case TokenEnum::Divide:
            TranslateBinaryOp(node, Operation::Divide);
            return;

        case TokenEnum::Mod:
            TranslateBinaryOp(node, Operation::Modulo);
            return;

        case TokenEnum::Or:
            TranslateBinaryOp(node, Operation::LogicalOr);
            return;

        case TokenEnum::And:
            TranslateBinaryOp(node, Operation::LogicalAnd);
            return;

        case TokenEnum::Int:
            KAI_TRACE() << "Translating Int: " << node->GetTokenText();
            Append(New<int>(boost::lexical_cast<int>(node->GetTokenText())));
            return;

        case TokenEnum::Float:
            KAI_TRACE() << "Translating Float: " << node->GetTokenText();
            Append(
                New<float>(boost::lexical_cast<float>(node->GetTokenText())));
            return;

        case TokenEnum::String:
            KAI_TRACE() << "Translating String: " << node->Text();
            // Do NOT wrap strings in Continuations
            Append(New<String>(node->Text()));
            KAI_TRACE() << "String translation complete";
            return;

        case TokenEnum::Ident:
            KAI_TRACE() << "Translating Ident: " << node->Text();
            // Do NOT wrap identifiers in Continuations
            Append(New<Label>(Label(node->Text())));
            KAI_TRACE() << "Ident translation complete";
            return;

        case TokenEnum::Pathname:
            KAI_TRACE() << "Translating Pathname: " << node->Text();
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

void RhoTranslator::TranslateBinaryOp(AstNodePtr node, Operation::Type op)
{
    KAI_TRACE() << "TranslateBinaryOp: Operation=" << Operation::ToString(op);

    TranslateNode(node->GetChild(0));
    TranslateNode(node->GetChild(1));
    AppendOp(op);

    KAI_TRACE() << "Binary operation successfully translated";
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

void RhoTranslator::TranslateNode(AstNodePtr node)
{
    if (!node) {
        Failed = true;
        return;
    }

    KAI_TRACE() << "TranslateNode: Type=" << RhoAstNodeEnumType::ToString(node->GetType())
               << " Text=" << node->Text();

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
            KAI_TRACE() << "Translating Assignment";
            // Put this in a try-catch block to help diagnose issues
            try {
                // Basic validation
                if (node->GetChildren().size() < 2) {
                    KAI_TRACE_ERROR() << "Assignment node has fewer than 2 children";
                    Fail("Assignment node has fewer than 2 children");
                    return;
                }

                // Value first (right side)
                TranslateNode(node->GetChild(1));

                // Then target (left side)
                TranslateNode(node->GetChild(0));

                // Add the store operation
                AppendOp(Operation::Store);

                KAI_TRACE() << "Completed assignment translation without Continuations";
            }
            catch (const std::exception& e) {
                KAI_TRACE_ERROR() << "Exception in assignment: " << e.what();
                Fail(std::string("Assignment failed: ") + e.what());
            }
            catch (...) {
                KAI_TRACE_ERROR() << "Unknown exception in assignment";
                Fail("Assignment failed with unknown exception");
            }
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
            for (auto st : node->GetChildren()) TranslateNode(st);
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

void RhoTranslator::TranslateBlock(AstNodePtr node)
{
    for (auto st : node->GetChildren()) TranslateNode(st);
}

void RhoTranslator::TranslateFunction(AstNodePtr node)
{
    // child 0: ident
    // child 1: args
    // child 2: block
    AstNode::ChildrenType const &ch = node->GetChildren();

    // In Pi, we need to create a Continuation for the function body
    Pointer<Continuation> cont = _reg->New<Continuation>();
    if (!cont.Exists()) {
        KAI_TRACE_ERROR() << "Failed to create function continuation";
        Fail("Failed to create function continuation");
        return;
    }

    // Set its code array
    cont->SetCode(_reg->New<Array>());
    if (!cont->GetCode().Exists()) {
        KAI_TRACE_ERROR() << "Failed to create function code array";
        Fail("Failed to create function code array");
        return;
    }

    // Write the body into the continuation's code array
    stack.push_back(cont);
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
    stack.pop_back();

    // Add the args
    for (auto a : ch[1]->GetChildren()) {
        cont->AddArg(Label(a->GetTokenText()));
    }

    // Write the name and store in this sequence: function object, function name, Store
    Append(cont);
    Append(New<Label>(Label(ch[0]->Text())));
    AppendOp(Operation::Store);
}

void RhoTranslator::TranslateCall(AstNodePtr node)
{
    KAI_TRACE() << "Translating call";

    typename AstNode::ChildrenType const &children = node->GetChildren();
    for (auto a : children[1]->GetChildren()) {
        TranslateNode(a);
    }

    TranslateNode(children[0]);

    Operation::Type callOp;
    if (children.size() > 2 && children[2]->GetToken().type == TokenEnum::Replace) {
        callOp = Operation::Replace;
    } else {
        callOp = Operation::Suspend;
    }

    AppendOp(callOp);

    KAI_TRACE() << "Completed call translation";
}

void RhoTranslator::TranslateIf(AstNodePtr node)
{
    KAI_TRACE() << "Translating if statement";

    typename AstNode::ChildrenType const &ch = node->GetChildren();
    bool hasElse = ch.size() > 2;

    // For if statements in Pi, we need to create continuations for then and else blocks
    Pointer<Continuation> thenCont = _reg->New<Continuation>();
    thenCont->SetCode(_reg->New<Array>());

    Pointer<Continuation> elseCont;
    if (hasElse) {
        elseCont = _reg->New<Continuation>();
        elseCont->SetCode(_reg->New<Array>());
    }

    // First translate the condition
    TranslateNode(ch[0]);

    // Translate then branch into its continuation
    stack.push_back(thenCont);
    TranslateNode(ch[1]);
    stack.pop_back();

    // Translate else branch if it exists
    if (hasElse) {
        stack.push_back(elseCont);
        TranslateNode(ch[2]);
        stack.pop_back();
    }

    // Add continuations and if operation
    Append(thenCont);

    Operation::Type ifOp;
    if (hasElse) {
        Append(elseCont);
        ifOp = Operation::IfThenSuspendElseSuspend;
    } else {
        ifOp = Operation::IfThenSuspend;
    }

    AppendOp(ifOp);

    KAI_TRACE() << "Completed if statement translation";
}

void RhoTranslator::TranslateWhile(AstNodePtr node)
{
    try {
        KAI_TRACE() << "Translating while loop";

        // Verify we have enough children
        if (node->GetChildren().size() < 1) {
            KAI_TRACE_ERROR() << "Not enough children in While node";
            Fail("While node must have at least a condition child");
            return;
        }

        // Get condition and body nodes
        AstNodePtr condition = node->GetChild(0);

        AstNodePtr body = nullptr;
        if (node->GetChildren().size() > 1) {
            body = node->GetChild(1);
        } else {
            // Create an empty body block node
            body = std::make_shared<RhoAstNode>(RhoAstNodeEnumType::Block);
        }

        // For while loops in Pi, we need continuations for condition and body
        // First create condition continuation
        Pointer<Continuation> condCont = _reg->New<Continuation>();
        condCont->SetCode(_reg->New<Array>());

        // Then create body continuation
        Pointer<Continuation> bodyCont = _reg->New<Continuation>();
        bodyCont->SetCode(_reg->New<Array>());

        // Translate condition into condition continuation
        stack.push_back(condCont);
        TranslateNode(condition);
        stack.pop_back();

        // Translate body into body continuation
        stack.push_back(bodyCont);
        TranslateNode(body);
        stack.pop_back();

        // Add continuations to code in correct order for WhileLoop operation
        Append(condCont);
        Append(bodyCont);

        // Add WhileLoop operation
        AppendOp(Operation::WhileLoop);

        KAI_TRACE() << "While loop translation complete";
    }
    catch (std::exception &e) {
        KAI_TRACE_ERROR() << "Exception in TranslateWhile: " << e.what();
        Fail(std::string("Exception in TranslateWhile: ") + e.what());
    }
    catch (...) {
        KAI_TRACE_ERROR() << "Unknown exception in TranslateWhile";
        Fail("Unknown exception in TranslateWhile");
    }
}

void RhoTranslator::TranslateDoWhile(AstNodePtr node)
{
    try {
        KAI_TRACE() << "Translating do-while loop";

        // Verify we have enough children
        if (node->GetChildren().size() < 2) {
            KAI_TRACE_ERROR() << "Not enough children in DoWhile node";
            Fail("DoWhile node must have both body and condition children");
            return;
        }

        // Get body and condition nodes (order is different from while loop)
        AstNodePtr body = node->GetChild(0);
        AstNodePtr condition = node->GetChild(1);

        // For do-while loops in Pi, we need continuations for condition and body
        // First create condition continuation
        Pointer<Continuation> condCont = _reg->New<Continuation>();
        condCont->SetCode(_reg->New<Array>());

        // Then create body continuation
        Pointer<Continuation> bodyCont = _reg->New<Continuation>();
        bodyCont->SetCode(_reg->New<Array>());

        // Translate body into body continuation
        stack.push_back(bodyCont);
        TranslateNode(body);
        stack.pop_back();

        // Translate condition into condition continuation
        stack.push_back(condCont);
        TranslateNode(condition);
        stack.pop_back();

        // Add continuations to code in correct order for DoLoop operation
        // Note: Order is different for do-while compared to while
        Append(condCont);
        Append(bodyCont);

        // Add DoLoop operation
        AppendOp(Operation::DoLoop);

        KAI_TRACE() << "Do-while loop translation complete";
    }
    catch (kai::Exception::Base &e) {
        KAI_TRACE_ERROR() << "KAI Exception in TranslateDoWhile: " << e.ToString();
        Fail(std::string("Exception in TranslateDoWhile: ") + e.ToString().c_str());
    }
    catch (std::exception &e) {
        KAI_TRACE_ERROR() << "Exception in TranslateDoWhile: " << e.what();
        Fail(std::string("Exception in TranslateDoWhile: ") + e.what());
    }
    catch (...) {
        KAI_TRACE_ERROR() << "Unknown exception in TranslateDoWhile";
        Fail("Unknown exception in TranslateDoWhile");
    }
}

KAI_END
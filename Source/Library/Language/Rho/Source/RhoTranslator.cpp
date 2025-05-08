#include <KAI/Core/BuiltinTypes.h>
#include <KAI/Executor/Operation.h>
#include <KAI/Language/Rho/RhoTranslator.h>

#include <boost/lexical_cast.hpp>
#include <boost/range/adaptor/reversed.hpp>

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
            
        case AstEnum::AcrossAllNodes:
            TranslateAcrossAllNodes(node);
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
    for (auto b : ch[2]->GetChildren()) TranslateNode(b);

    // add the args
    auto cont = Pop();
    for (auto a : ch[1]->GetChildren()) cont->AddArg(Label(a->GetTokenText()));

    // write the _name and store
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

void RhoTranslator::TranslateFor(AstNodePtr node) { 
    KAI_TRACE() << "Starting to translate For construct";
    KAI_TRACE() << "Node: " << node->ToString();
    
    // Check the number of children
    int numChildren = node->GetChildren().size();
    KAI_TRACE() << "Number of children: " << numChildren;
    
    if (numChildren < 2) {
        KAI_TRACE_ERROR() << "For loop needs at least initialization and condition";
        Failed = true;
        return;
    }
    
    if (numChildren == 2) {
        // This is a for-each style loop: for item in collection
        AstNodePtr item = node->GetChild(0);
        AstNodePtr collection = node->GetChild(1);
        
        KAI_TRACE() << "Translating for-each loop";
        KAI_TRACE() << "Item: " << item->ToString() << ", Collection: " << collection->ToString();
        
        // Translate the collection
        TranslateNode(collection);
        
        // Create a label for the item
        Append(New<Label>(Label(item->Text())));
        
        // Add the foreach operation
        AppendOp(Operation::ForEach);
        
        // If there's a body (block as 3rd child), translate it
        if (numChildren > 2) {
            TranslateNode(node->GetChild(2));
            // ForEachNext operation is not defined, use a comment for now
            // AppendOp(Operation::ForEachNext);
        }
    }
    else if (numChildren >= 3) {
        // This is a C-style for loop: for init; condition; increment
        KAI_TRACE() << "Translating C-style for loop";
        
        // Translate initialization
        AstNodePtr init = node->GetChild(0);
        KAI_TRACE() << "Initialization: " << init->ToString();
        TranslateNode(init);
        
        // Create a continuation for the condition
        PushNew();
        
        // Translate condition
        AstNodePtr condition = node->GetChild(1);
        KAI_TRACE() << "Condition: " << condition->ToString();
        TranslateNode(condition);
        
        // If there's an increment expression (3rd child)
        if (numChildren > 2) {
            AstNodePtr increment = node->GetChild(2);
            KAI_TRACE() << "Increment: " << increment->ToString();
            TranslateNode(increment);
        }
        
        // If there's a body (4th child), translate it
        if (numChildren > 3) {
            AstNodePtr body = node->GetChild(3);
            KAI_TRACE() << "Body: " << body->ToString();
            TranslateNode(body);
        }
        
        // Append the ForLoop operation with the continuation
        auto cont = Pop();
        Append(cont);
        AppendOp(Operation::ForLoop);
    }
    
    KAI_TRACE() << "Finished translating For construct";
}

void RhoTranslator::TranslateAcrossAllNodes(AstNodePtr node) {
    KAI_TRACE() << "Starting to translate AcrossAllNodes construct";
    size_t numChildren = node->GetChildren().size();
    KAI_TRACE() << "Node has " << (int)numChildren << " children";
    
    if (numChildren != 3) {
        KAI_TRACE_ERROR() << "Error: AcrossAllNodes requires exactly 3 children but has " << 
                         (int)numChildren;
        Fail("AcrossAllNodes requires exactly 3 arguments");
        return;
    }
    
    // The executor's AcrossAllNodes operation expects the arguments in this order:
    // 1. Network node (or null for local execution)
    // 2. Collection to iterate over
    // 3. Function to apply to each element
    
    try {
        // Network node argument (child 0)
        AstNodePtr networkNode = node->GetChild(0);
        KAI_TRACE() << "Translating network node argument";
        TranslateNode(networkNode);
        
        // Collection argument (child 1)
        AstNodePtr collectionNode = node->GetChild(1);
        KAI_TRACE() << "Translating collection argument";
        TranslateNode(collectionNode);
        
        // Function argument (child 2)
        AstNodePtr functionNode = node->GetChild(2);
        KAI_TRACE() << "Translating function argument";
        TranslateNode(functionNode);
        
        // Append the AcrossAllNodes operation
        KAI_TRACE() << "Appending AcrossAllNodes operation";
        AppendOp(Operation::AcrossAllNodes);
    }
    catch (std::exception &e) {
        KAI_TRACE_ERROR() << "Exception in TranslateAcrossAllNodes: " << e.what();
        Fail(std::string("Exception in AcrossAllNodes: ") + e.what());
    }
    
    KAI_TRACE() << "Finished translating AcrossAllNodes construct";
}

void RhoTranslator::TranslateWhile(AstNodePtr node) {
    KAI_TRACE() << "Starting to translate While construct";
    KAI_TRACE() << "Node: " << node->ToString();
    KAI_TRACE() << "Node has " << (int)node->GetChildren().size() << " children";
    
    // Simpler implementation - create very straightforward Continuations
    try {
        // Verify we have enough children
        if (node->GetChildren().size() < 1) {
            KAI_TRACE_ERROR() << "While node must have at least a condition child";
            Fail("While node must have at least a condition child");
            return;
        }
        
        // Get condition and body nodes
        AstNodePtr condition = node->GetChild(0);
        KAI_TRACE() << "Got condition node: " << condition->ToString();
        
        AstNodePtr body = nullptr;
        if (node->GetChildren().size() > 1) {
            body = node->GetChild(1);
            KAI_TRACE() << "Got body node: " << body->ToString();
        } else {
            KAI_TRACE_WARN() << "No body found for while loop, creating empty body";
            // Create an empty body block node
            body = std::make_shared<RhoAstNode>(RhoAstNodeEnumType::Block);
        }
        
        // Create a basic continuation for the condition
        KAI_TRACE() << "Creating condition continuation";
        PushNew();
        TranslateNode(condition);
        Object condCont = Pop();
        
        if (!condCont.GetClass()) {
            KAI_TRACE_ERROR() << "Condition continuation has no class!";
            Fail("Condition continuation has no class");
            return;
        }
        
        KAI_TRACE() << "Condition continuation type: " << condCont.GetClass()->GetName();
        
        // Create a basic continuation for the body
        KAI_TRACE() << "Creating body continuation";
        PushNew();
        TranslateNode(body);
        Object bodyCont = Pop();
        
        if (!bodyCont.GetClass()) {
            KAI_TRACE_ERROR() << "Body continuation has no class!";
            Fail("Body continuation has no class");
            return;
        }
        
        KAI_TRACE() << "Body continuation type: " << bodyCont.GetClass()->GetName();
        
        // Push in the correct order for the executor
        // The executor pops body first, then condition
        KAI_TRACE() << "Pushing continuations to stack:";
        KAI_TRACE() << "  First: test continuation (popped second)";
        Append(condCont);
        KAI_TRACE() << "  Second: body continuation (popped first)";
        Append(bodyCont);
        
        // Add WhileLoop operation
        KAI_TRACE() << "Adding WhileLoop operation";
        AppendOp(Operation::WhileLoop);
        
        KAI_TRACE() << "TranslateWhile completed successfully";
    }
    catch (std::exception &e) {
        KAI_TRACE_ERROR() << "Exception in TranslateWhile: " << e.what();
        Fail(std::string("Exception in TranslateWhile: ") + e.what());
    }
}

KAI_END

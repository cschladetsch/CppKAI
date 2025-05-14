#include <KAI/Core/Config/Base.h>
#include <KAI/Core/Registry.h>
#include <KAI/Language/Common/TranslatorCommon.h>

#include <iostream>

KAI_BEGIN

TranslatorCommon::TranslatorCommon(Registry &r) : ProcessCommon(r) {}

void TranslatorCommon::Append(Object const &ob) {
    try {
        if (stack.empty()) {
            KAI_TRACE_ERROR() << "TranslatorCommon::Append: Stack is empty";
            KAI_THROW_0(EmptyStack);
        }

        auto top = Top();
        if (!top.Exists()) {
            KAI_TRACE_ERROR()
                << "TranslatorCommon::Append: Top of stack is invalid";
            KAI_THROW_0(NullObject);
        }

        auto code = top->GetCode();
        if (!code.Exists()) {
            KAI_TRACE_ERROR()
                << "TranslatorCommon::Append: Code array is invalid";
            KAI_THROW_0(NullObject);
        }

        KAI_TRACE() << "TranslatorCommon::Append: " << ob.ToString();
        code->Append(ob);
    } catch (kai::Exception::Base &e) {
        KAI_TRACE_ERROR() << "Exception in TranslatorCommon::Append: "
                          << e.ToString();
        throw;
    } catch (std::exception &e) {
        KAI_TRACE_ERROR() << "Exception in TranslatorCommon::Append: "
                          << e.what();
        throw;
    } catch (...) {
        KAI_TRACE_ERROR() << "Unknown exception in TranslatorCommon::Append";
        throw;
    }
}

void TranslatorCommon::AppendOp(Operation::Type op) {
    std::cout << "Appending operation: " << Operation::ToString(op)
              << std::endl;

    // Create a new Operation object and add it directly to the code array
    Object opObject = reg_->New<Operation>(op);

    // The operation should be added directly to the current continuation's code
    // array
    Append(opObject);
}

void TranslatorCommon::AppendDirectOperation(Operation::Type op) {
    KAI_TRACE() << "Info: TranslatorCommon::AppendDirectOperation: " << Operation::ToString(op);
    
    // This method creates and adds an operation directly to the parent stack
    // instead of wrapping it in another continuation
    // This is used for Rho language binary operations to avoid unnecessary nesting
    
    Object opObject = reg_->New<Operation>(op);
    
    // Add directly to the current continuation's code array
    if (stack.empty()) {
        KAI_TRACE_ERROR() << "TranslatorCommon::AppendDirectOperation: Stack is empty";
        return; // Instead of throwing, just return
    }
    
    auto top = Top();
    if (!top.Exists()) {
        KAI_TRACE_ERROR() << "TranslatorCommon::AppendDirectOperation: Top of stack is invalid";
        return; // Instead of throwing, just return
    }
    
    auto code = top->GetCode();
    if (!code.Exists()) {
        KAI_TRACE_ERROR() << "TranslatorCommon::AppendDirectOperation: Code array is invalid";
        return; // Instead of throwing, just return
    }
    
    code->Append(opObject);
}

void TranslatorCommon::MarkAsRhoExpression() {
    if (stack.empty()) {
        KAI_TRACE_ERROR() << "TranslatorCommon::MarkAsRhoExpression: Stack is empty";
        return; // Instead of throwing, just return
    }
    
    auto top = Top();
    if (!top.Exists()) {
        KAI_TRACE_ERROR() << "TranslatorCommon::MarkAsRhoExpression: Top of stack is invalid";
        return; // Instead of throwing, just return
    }
    
    // We'll skip setting properties for now since the Continuation class
    // doesn't have the RhoExpression property registered correctly
    // This will be implemented in a future PR when property handling is fixed
    
    // Instead, we'll rely on the language context in Console.cpp
    // to determine how to process continuations
    
    // Original code commented out to prevent errors:
    // top.SetPropertyValue(Label("RhoExpression"), reg_->New<bool>(true));
}

Pointer<Continuation> TranslatorCommon::Top() { return stack.back(); }

void TranslatorCommon::PushNew() {
    Pointer<Continuation> c = reg_->New<Continuation>();
    c->SetCode(reg_->New<Array>());
    stack.push_back(c);
}

Pointer<Continuation> TranslatorCommon::Pop() {
    auto top = Top();
    stack.pop_back();
    return top;
}

std::string TranslatorCommon::ToString() const {
    StringStream str;
    for (auto ob : *stack.back()->GetCode()) str << ' ' << ob;
    return str.ToString().c_str();
}

KAI_END

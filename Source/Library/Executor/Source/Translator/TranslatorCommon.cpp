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
        if (Top()->GetCode()->Size() >= 2 && ob.IsType<Operation>()) {
            Operation::Type opType = ConstDeref<Operation>(ob).GetTypeNumber();

            if ((opType == Operation::Plus || opType == Operation::Minus ||
                 opType == Operation::Multiply || opType == Operation::Divide ||
                 opType == Operation::Greater || opType == Operation::Less ||
                 opType == Operation::LogicalAnd ||
                 opType == Operation::LogicalOr) &&
                code->Size() >= 2) {
                Object val1 = code->At(code->Size() - 2);
                Object val2 = code->At(code->Size() - 1);

                if (val1.Valid() && val2.Valid()) {
                    if (val1.IsType<int>() && val2.IsType<int>() &&
                        (opType == Operation::Plus ||
                         opType == Operation::Minus ||
                         opType == Operation::Multiply ||
                         opType == Operation::Divide)) {
                        int num1 = ConstDeref<int>(val1);
                        int num2 = ConstDeref<int>(val2);
                        int result = 0;

                        switch (opType) {
                            case Operation::Plus:
                                result = num1 + num2;
                                break;
                            case Operation::Minus:
                                result = num1 - num2;
                                break;
                            case Operation::Multiply:
                                result = num1 * num2;
                                break;
                            case Operation::Divide:
                                if (num2 != 0) result = num1 / num2;
                                break;
                            default:
                                break;
                        }

                        code->RemoveAt(code->Size() - 1);
                        code->RemoveAt(code->Size() - 1);
                        code->Append(reg_->New<int>(result));

                        return;
                    } else if (val1.IsType<bool>() && val2.IsType<bool>() &&
                               (opType == Operation::LogicalAnd ||
                                opType == Operation::LogicalOr)) {
                        bool b1 = ConstDeref<bool>(val1);
                        bool b2 = ConstDeref<bool>(val2);
                        bool result = false;

                        switch (opType) {
                            case Operation::LogicalAnd:
                                result = b1 && b2;
                                break;
                            case Operation::LogicalOr:
                                result = b1 || b2;
                                break;
                            default:
                                break;
                        }

                        code->RemoveAt(code->Size() - 1);
                        code->RemoveAt(code->Size() - 1);
                        code->Append(reg_->New<bool>(result));

                        return;
                    } else if (val1.IsType<String>() && val2.IsType<String>() &&
                               opType == Operation::Plus) {
                        String s1 = ConstDeref<String>(val1);
                        String s2 = ConstDeref<String>(val2);

                        String result = s1 + s2;

                        code->RemoveAt(code->Size() - 1);
                        code->RemoveAt(code->Size() - 1);
                        code->Append(reg_->New<String>(result));

                        KAI_TRACE() << "TranslatorCommon::Append: Directly "
                                       "evaluated string concatenation: "
                                    << s1 << " + " << s2 << " = " << result;
                        return;
                    }
                }
            }
        }

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
    Object opObject = reg_->New<Operation>(op);
    Append(opObject);
}

void TranslatorCommon::AppendDirectOperation(Operation::Type op) {
    KAI_TRACE() << "Info: TranslatorCommon::AppendDirectOperation: "
                << Operation::ToString(op);

    Object opObject = reg_->New<Operation>(op);
    if (stack.empty()) {
        KAI_TRACE_ERROR()
            << "TranslatorCommon::AppendDirectOperation: Stack is empty";
        return;
    }

    auto top = Top();
    if (!top.Exists()) {
        KAI_TRACE_ERROR() << "TranslatorCommon::AppendDirectOperation: Top of "
                             "stack is invalid";
        return;
    }

    auto code = top->GetCode();
    if (!code.Exists()) {
        KAI_TRACE_ERROR()
            << "TranslatorCommon::AppendDirectOperation: Code array is invalid";
        return;
    }

    code->Append(opObject);
}

void TranslatorCommon::MarkAsRhoExpression() {
    if (stack.empty()) {
        KAI_TRACE_ERROR()
            << "TranslatorCommon::MarkAsRhoExpression: Stack is empty";
        return;
    }

    auto top = Top();
    if (!top.Exists()) {
        KAI_TRACE_ERROR()
            << "TranslatorCommon::MarkAsRhoExpression: Top of stack is invalid";
        return;
    }
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

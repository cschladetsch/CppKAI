#include <KAI/Core/Config/Base.h>
#include <KAI/Core/Registry.h>
#include <KAI/Language/Common/TranslatorCommon.h>
#include <iostream>

KAI_BEGIN

TranslatorCommon::TranslatorCommon(Registry &r) : ProcessCommon(r) {}

void TranslatorCommon::Append(Object const &ob) {
    Top()->GetCode()->Append(ob);
}

void TranslatorCommon::AppendOp(Operation::Type op) {
    std::cout << "Appending operation: " << Operation::ToString(op) << std::endl;
    AppendNew(Operation(op));
}

Pointer<Continuation> TranslatorCommon::Top() { return stack.back(); }

void TranslatorCommon::PushNew() {
    Pointer<Continuation> c = _reg->New<Continuation>();
    c->SetCode(_reg->New<Array>());
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

#include <KAI/Executor/Operation.h>
#include <KAI/Language/Common/TranslatorCommon.h>

#include <iostream>

KAI_BEGIN

void TranslatorCommon::AppendDirectOperation(Operation::Type op) {
    std::cout << "Appending direct operation: " << Operation::ToString(op)
              << std::endl;

    // Create the operation as a raw object, no continuation wrapping
    Object opObject = _reg->New<Operation>(op);
    Append(opObject);
}

KAI_END
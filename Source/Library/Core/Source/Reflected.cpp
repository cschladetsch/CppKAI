#include <KAI/Core/Config/Base.h>
#include <KAI/Core/Object.h>

KAI_BEGIN

Registry &Reflected::Reg() const {
    if (!Self || !Self->Exists()) KAI_THROW_0(NullObject);

    return *Self->GetRegistry();
}

KAI_END

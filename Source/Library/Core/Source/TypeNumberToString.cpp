#include <KAI/Core/BuiltinTypes/Signed32.h>
#include <KAI/Core/StringStream.h>
#include <KAI/Core/Type/Number.h>

KAI_BEGIN

namespace Type {
std::string Number::ToString() const {
    switch (value) {
#define Case(N) \
    case N:     \
        return #N;
        Case(Undefined);
        Case(None);
        Case(List);
        Case(Void);
        Case(Handle);
        Case(Object);
        Case(Type);
        Case(TypeNumber);
        Case(BasePointer);
        Case(Callable);
        Case(Function);
        Case(Method);
        Case(Property);
        Case(Class);
        Case(Bool);
        Case(Signed32);
        Case(Single);
        Case(Double);
        Case(String);
        Case(Label);
        Case(Pathname);
        Case(Array);
        Case(Pair);
        Case(Map);
        Case(HashMap);
        Case(Set);
        Case(HashSet);
        Case(Stack);
        Case(MapConstIterator);
        Case(MapIterator);
        Case(SetConstIterator);
        Case(SetIterator);
        Case(HashMapConstIterator);
        Case(HashMapIterator);
        Case(HashSetConstIterator);
        Case(HashSetIterator);
        Case(Operation);
        Case(Continuation);
        Case(Compiler);
        Case(Executor);
        Case(StreamBase);
        Case(StringStream);
        Case(XmlStream);
        Case(GraphStream);
        Case(BinaryStream);
        Case(Console);
        Case(Node);
        Case(Browser);
        Case(SignedContinuation);
        Case(Vector2);
        Case(Vector3);
        Case(Vector4);
        Case(Last);
        Case(SystemLast);
    }

    KAI_NAMESPACE(StringStream) S;
    S << "TypeNumber=" << (int)value << Ends;
    return S.ToString().c_str();
}
}  // namespace Type

KAI_END

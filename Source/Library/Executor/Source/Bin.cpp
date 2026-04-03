#include <fstream>
#include <iostream>

#include "KAI/Core/BuiltinTypes/Pair.h"
#include "KAI/Core/BuiltinTypes/Vector4.h"
#include "KAI/Core/BuiltinTypes/Void.h"
#include "KAI/Core/Detail/Function.h"
#include "KAI/Core/Exception.h"
#include "KAI/Executor/Compiler.h"
#include "KAI/UnfuckWin32.h"

KAI_BEGIN

using namespace std;

bool ExecuteFile(const char *filename, Pointer<Executor> executor,
                 Pointer<Compiler> compiler, Object scope);

namespace Bin {
Object GetMethods(Object Q) {
    if (!Q.Exists()) return Object();

    auto &r = *Q.GetRegistry();
    Value<Array> methods = r.New<Array>();
    const ClassBase *K = Q.GetClass();
    for (auto m : K->GetMethods())
        methods->Append(r.New<BasePointer<MethodBase>>(m.second));

    return methods.GetObject();
}

void Help() { cout << "type /Bin ls" << endl; }

Object GetProperties(Object Q) {
    if (!Q.Exists()) return Object();

    auto &r = *Q.GetRegistry();
    Value<Array> properties = r.New<Array>();
    for (auto prop : Q.GetClass()->GetProperties())
        properties->Append(r.New<BasePointer<PropertyBase>>(prop.second));

    return properties.GetObject();
}

Vector3 ScaleVector3(Vector3 vec, float scalar) {
    return Vector3(vec.x * scalar, vec.y * scalar, vec.z * scalar);
}

Vector3 AddVector3(Vector3 vec, Vector3 addition) {
    return Vector3(vec.x + addition.x, vec.y + addition.y, vec.z + addition.z);
}

void WriteToFile(String filename, Object Q) {
    std::fstream file(filename.c_str(), std::ios::out);
    file << Q.ToString().c_str();
    file.close();
}

String ReadFile(String filename) {
    std::fstream file(filename.c_str(), std::ios::in);
    if (!file) KAI_THROW_1(FileNotFound, filename);

    char line[2000];
    StringStream text;
    while (file.getline(line, 2000)) {
        text.Append(std::string_view(line));
        text.Append('\n');
    }

    file.close();
    return text.ToString();
}

void Printf(String fmt, Array items) {
    for (int n = 0; n < fmt.size(); ++n) {
        char ch = fmt[n];
        if (ch == '#') {
            int num = fmt[++n] - '0';
            StringStream str;
            str << items.At(num);
            std::cout << str.ToString().c_str();
            continue;
        }

        std::cout << ch;
    }
}

void Print(Object Q) { std::cout << Q.ToString().c_str(); }

void print(Object Q) { std::cout << Q.ToString().c_str() << std::endl; }

void PrintXml(Object Q) { std::cout << Q.ToXmlString().c_str() << std::endl; }

Object UpCast(Object Q) {
    if (!Q.Exists()) return Object();

    return Q.GetClass()->UpCast(Q.GetStorageBase());
}

void SetClean(Object Q, bool D) {
    if (D)
        Q.SetClean();
    else
        Q.SetClean(false);
}

bool IsDirty(Object Q) { return Q.IsDirty(); }

bool IsConst(Object Q) { return Q.IsConst(); }

void Assert(bool B) {
    if (!B) KAI_THROW_0(AssertionFailed);
}

Object Freeze(Object Q) {
    Value<BinaryStream> S = Q.GetRegistry()->New<BinaryStream>();
    *S << Q;
    return S.GetObject();
}

Object Thaw(Object Q) {
    BinaryStream &S = Deref<BinaryStream>(Q);
    S.SetRegistry(Q.GetRegistry());
    Object N;
    S >> N;
    return N;
}

String ToString(Object Q) { return Q.ToString(); }

String ToXmlString(Object Q) { return Q.ToXmlString(); }

void ExitToOS(int N) { exit(N); }

/*Pointer<ClassBase const *> */ Object TypeNumberToClass(Object tn) {
    Pointer<int> type_number = tn;
    return type_number.GetRegistry()->New(
        type_number.GetRegistry()->GetClass(*type_number));
}

/// returns a pair containing the methods_ and properties_ of a class
/*Pointer<Pair>*/ Object Describe(Object Q) {
    Pointer<Pair> P = Q.GetRegistry()->New<Pair>();
    P->first = GetMethods(Q);
    P->second = GetProperties(Q);
    return P;
}

String info(Object object) {
    StringStream text;
    text << "Methods:\n";
    for (auto const &method : ConstDeref<Array>(GetMethods(object)))
        text << "\t" << method.ToString() << "\n";

    text << "Properties:\n";
    for (auto const &prop : ConstDeref<Array>(GetProperties(object)))
        text << "\t" << prop.ToString() << "\n";

    return text.ToString();
}

void AddFunctions(Object Q) {
#define ADD_FUN(N, D) AddFunction(Q, N, Label(#N), D);
    ADD_FUN(TypeNumberToClass, "Returns the Class of a given type number");
    ADD_FUN(GetMethods, "Returns an Array of all Methods for a given Object");
    ADD_FUN(GetProperties,
            "Returns an Array of all Properties for a given Object");
    ADD_FUN(Describe, "Returns Methods and Properties for a given Object");
    ADD_FUN(info, "Prints the methods_ and properties_");
    AddFunction(Q, ExitToOS, Label("Exit"), "Ends the Session");
    ADD_FUN(Assert, "Throws AssertionFailed if Object equates to False");
    ADD_FUN(Freeze, "Inserts Object into a BinaryStream");
    ADD_FUN(Thaw, "Extracts an Object from a BinarySteam");
    ADD_FUN(IsDirty, "Returns True if Object is Dirty");
    ADD_FUN(IsConst, "Returns True if Object is Const");
    ADD_FUN(SetClean, "");
    ADD_FUN(ToString,
            "Returns a human-readable String reprentation of an Object");
    ADD_FUN(ToXmlString, "Returns an XML String reprentation of an Object");
    ADD_FUN(UpCast, "Returns a Pointer to the Base of an Object");
    ADD_FUN(Print, "Prints an Object to stdout");
    ADD_FUN(print, "Prints an Object to KAI info log");
    ADD_FUN(Printf, "Print a list of items");
    ADD_FUN(PrintXml, "Print's an Object to stdout in XML");
    ADD_FUN(WriteToFile, "Writes an Object to a text file");
    ADD_FUN(ReadFile, "Reads a text file");
    // No Traits for Vector3:
    // ADD_FUN(ScaleVector3, "Multiply a vector by a float");
    ADD_FUN(AddVector3, "Adds two Vector3s together.")
    ADD_FUN(Help, "LOL");
}
}  // namespace Bin

KAI_END

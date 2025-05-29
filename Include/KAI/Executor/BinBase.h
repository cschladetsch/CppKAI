#pragma once

#include <KAI/Core/BuiltinTypes/Array.h>
#include <KAI/Core/BuiltinTypes/String.h>
#include <KAI/Core/BuiltinTypes/Vector4.h>
#include <KAI/Core/Config/Base.h>
#include <KAI/Core/Object/Object.h>

KAI_BEGIN

/// Defines what goes into the /bin folder at runtime setup
namespace Bin {
void Help();
Object GetMethods(Object Q);
Object GetProperties(Object Q);
Vector3 ScaleVector3(Vector3 vec, float scalar);
Vector3 AddVector3(Vector3 vec, Vector3 addition);
void WriteToFile(String filename, Object Q);
String ReadFile(String filename);
void Printf(String fmt, Array items);
void Print(Object Q);
void print(Object Q);
void PrintXml(Object Q);
Object UpCast(Object Q);
void SetClean(Object Q, bool D);
bool IsDirty(Object Q);
bool IsClean(Object Q);
bool IsConst(Object Q);
void Assert(bool B);
Object Freeze(Object Q);
Object Thaw(Object Q);
String ToString(Object Q);
String ToXmlString(Object Q);
String Version();
void Quit();
void ExitToOS(int N);
Object RunOne(Object object);
Object RunAllTests(Object object);
Object TypeNumberToClass(Object tn);
Object Describe(Object Q);
String info(Object object);
void AddFunctions(Object Q);
}  // namespace Bin

KAI_END

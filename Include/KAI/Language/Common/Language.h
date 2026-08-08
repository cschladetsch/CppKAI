#pragma once

KAI_BEGIN

class PiTranslator;
class RhoTranslator;
class LispTranslator;

// Appended rather than inserted: Compiler stores the active language as an int
// and Console::SetLanguage(int) is part of the public API, so the existing
// values have to keep their numbers.
enum class Language { None, Pi, Rho, Tau, Hlsl, Sigma, Lisp };
const char *ToString(Language);

KAI_END

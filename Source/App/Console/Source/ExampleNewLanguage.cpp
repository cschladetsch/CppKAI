// Example: How to add a new language to the KAI Console
// This file demonstrates how to integrate a new language translator

#include "KAI/Console/Console.h"
#include "KAI/Language/Common/TranslatorFactory.h"
#include "KAI/Language/Sigma/SigmaTranslator.h"

using namespace kai;

// Register the Sigma translator
// This line automatically registers the translator with the factory
// when the program starts
REGISTER_TRANSLATOR(Language::Sigma, SigmaTranslator)

// Example usage:
void ExampleUsage() {
    // Create a console
    Console console;

    // Set the language to Sigma
    console.SetLanguage(Language::Sigma);

    // Create and set the Sigma translator
    auto& reg = console.GetRegistry();
    auto sigmaTranslator =
        TranslatorFactory::Instance().CreateTranslator(Language::Sigma, reg);

    if (sigmaTranslator) {
        console.SetTranslator(sigmaTranslator);

        // Now the console is ready to process Sigma language code
        // console.Process("sigma code here");
    }
}

// To add a new language:
// 1. Create the language translator class (e.g., SigmaTranslator)
//    - Inherit from TranslatorBase<YourParser>
//    - Implement the parser and lexer
//    - Implement TranslateNode to convert AST to KAI bytecode
//
// 2. Add the language to the Language enum in Language.h
//
// 3. Update the ToString function in Language.cpp
//
// 4. Register the translator using REGISTER_TRANSLATOR macro
//
// 5. The language is now available in the console!
//    - Users can switch to it using console commands
//    - Files with the appropriate extension will be processed correctly
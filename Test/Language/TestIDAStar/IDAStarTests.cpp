#include <KAI/Console/Console.h>
#include <KAI/Language/Common/Language.h>
#include <gtest/gtest.h>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "TestLangCommon.h"

using namespace kai;

#ifndef KAI_IDASTAR_SCRIPTS
#define KAI_IDASTAR_SCRIPTS "."
#endif

// Runs the same IDA* search written three times -- once per language -- over
// one grid, and checks they agree.
//
// The programs live in Scripts/ and are read at run time rather than compiled
// in, so they can be edited and re-run without rebuilding.
struct IDAStarTests : TestLangCommon {
    static std::string ReadScript(const std::string &name) {
        const std::string path = std::string(KAI_IDASTAR_SCRIPTS) + "/" + name;
        std::ifstream in(path);
        if (!in) return "";
        std::stringstream ss;
        ss << in.rdbuf();
        return ss.str();
    }

    // Executes a script and returns whatever it left on top of the stack.
    Object Run(Language lang, const std::string &name) {
        const std::string text = ReadScript(name);
        if (text.empty()) {
            ADD_FAILURE() << "Could not read script: " << name;
            return Object();
        }

        console_.SetLanguage(lang);
        data_->Clear();
        console_.Execute(text.c_str(), Structure::Program);

        if (data_->Empty()) return Object();
        return data_->Top();
    }

    // Each program finishes with a two element array: the optimal path cost
    // (-1 when none was found inside the node budget) and the number of nodes
    // expanded getting there.
    struct Outcome {
        int cost = -2;
        int nodes = -1;
        bool valid = false;
    };

    Outcome RunSearch(Language lang, const std::string &name) {
        Outcome out;
        Object result = Run(lang, name);
        if (!result.Exists()) return out;

        if (!result.IsType<Array>()) {
            ADD_FAILURE() << name << ": expected [cost, nodes], got "
                          << (result.GetClass()
                                  ? result.GetClass()->GetName().ToString()
                                  : "<null>");
            return out;
        }

        auto &arr = Deref<Array>(result);
        if (arr.Size() != 2) {
            ADD_FAILURE() << name << ": expected 2 elements, got "
                          << arr.Size();
            return out;
        }

        out.cost = ConstDeref<int>(arr.At(0));
        out.nodes = ConstDeref<int>(arr.At(1));
        out.valid = true;
        return out;
    }
};

// A scratch slot for trying a snippet without a rebuild:
//   KAI_PROBE=probe.rho KAI_PROBE_LANG=rho ./Bin/Test/TestIDAStar \
//       --gtest_filter=IDAStarTests.Probe
TEST_F(IDAStarTests, Probe) {
    const char *file = std::getenv("KAI_PROBE");
    if (file == nullptr) GTEST_SKIP() << "set KAI_PROBE to use this";

    const char *langName = std::getenv("KAI_PROBE_LANG");
    Language lang = Language::Rho;
    if (langName != nullptr) {
        const std::string l(langName);
        if (l == "pi") lang = Language::Pi;
        if (l == "lisp") lang = Language::Lisp;
    }

    Object result = Run(lang, file);
    std::cout << "PROBE result: ";
    if (!result.Exists()) {
        std::cout << "<nothing on stack>" << std::endl;
    } else {
        std::cout << result.ToString().c_str() << "  (type "
                  << (result.GetClass()
                          ? result.GetClass()->GetName().ToString()
                          : "<null>")
                  << ")" << std::endl;
    }
    std::cout << "PROBE stack depth: " << data_->Size() << std::endl;
}

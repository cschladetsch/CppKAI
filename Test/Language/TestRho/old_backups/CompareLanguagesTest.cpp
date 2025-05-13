#include <gtest/gtest.h>

#include <iostream>
#include <string>

#include "KAI/Core/Console.h"

using namespace kai;
using namespace std;

// A test that compares Pi and Rho language arithmetic with detailed trace
// output
TEST(LanguageComparison, DISABLED_ArithmeticRhoVsPi) {
    cout << "=========== PI LANGUAGE TEST ===========" << endl;

    // Create Pi console
    Console piConsole;
    piConsole.SetLanguage(Language::Pi);

    // Register basic types
    Registry& piReg = piConsole.GetRegistry();
    piReg.AddClass<int>(Label("int"));

    auto piExec = piConsole.GetExecutor();
    piExec->SetTraceLevel(5);  // More verbose trace

    try {
        // Pi language test
        cout << "Executing Pi: '2 3 +'" << endl;
        auto piStack = piExec->GetDataStack();
        piStack->Clear();

        piConsole.Execute("2 3 +");

        ASSERT_FALSE(piStack->Empty());
        Object piResult = piStack->Top();
        ASSERT_TRUE(piResult.IsType<int>());
        ASSERT_EQ(ConstDeref<int>(piResult), 5);

        cout << "Pi test passed: 2 3 + = " << ConstDeref<int>(piResult) << endl;

    } catch (const Exception::Base& e) {
        cerr << "Pi KAI Exception: " << e.ToString() << endl;
        FAIL() << "Pi test failed with KAI exception: " << e.ToString();
    }

    cout << "=========== RHO LANGUAGE TEST ===========" << endl;

    // Create Rho console
    Console rhoConsole;
    rhoConsole.SetLanguage(Language::Rho);

    // Register basic types
    Registry& rhoReg = rhoConsole.GetRegistry();
    rhoReg.AddClass<int>(Label("int"));

    auto rhoExec = rhoConsole.GetExecutor();
    rhoExec->SetTraceLevel(5);  // More verbose trace

    try {
        // Rho language test
        cout << "Executing Rho: '2 + 3'" << endl;
        auto rhoStack = rhoExec->GetDataStack();
        rhoStack->Clear();

        rhoConsole.Execute("2 + 3");

        ASSERT_FALSE(rhoStack->Empty());
        Object rhoResult = rhoStack->Top();

        cout << "Stack contents after execution:" << endl;
        rhoExec->PrintStack(cout);

        ASSERT_TRUE(rhoResult.IsType<int>());
        ASSERT_EQ(ConstDeref<int>(rhoResult), 5);

        cout << "Rho test passed: 2 + 3 = " << ConstDeref<int>(rhoResult)
             << endl;

    } catch (const Exception::Base& e) {
        cerr << "Rho KAI Exception: " << e.ToString() << endl;
        FAIL() << "Rho test failed with KAI exception: " << e.ToString();
    }
}
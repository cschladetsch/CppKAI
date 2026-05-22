#include <gtest/gtest.h>

#include "KAI/Console/Console.h"

TEST(IntegratedConsoleTest, BasicConnectionTest) {
    kai::Console server;
    kai::Console client;

    server.SetLanguage(kai::Language::Pi);
    client.SetLanguage(kai::Language::Pi);

    ASSERT_NO_THROW(server.Execute("1 2 +"));
    ASSERT_NO_THROW(client.Execute("10 3 -"));

    auto serverStack = server.GetExecutor()->GetDataStack();
    auto clientStack = client.GetExecutor()->GetDataStack();

    ASSERT_EQ(serverStack->Size(), 1);
    ASSERT_EQ(clientStack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(serverStack->Top()), 3);
    EXPECT_EQ(kai::ConstDeref<int>(clientStack->Top()), 7);
}

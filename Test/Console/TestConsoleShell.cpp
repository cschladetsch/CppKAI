#include <gtest/gtest.h>

#include <filesystem>
#include <string>

#include "KAI/Console/Console.h"

namespace {

std::string TrimTrailingNewline(std::string text) {
    while (!text.empty() && (text.back() == '\n' || text.back() == '\r')) {
        text.pop_back();
    }
    return text;
}

}  // namespace

TEST(ConsoleShell, CdPersistsAcrossShellCommands) {
    kai::Console console;
    const auto original = std::filesystem::current_path();
    const auto temp = std::filesystem::temp_directory_path();

    EXPECT_EQ(console.Process(kai::String("$ cd /")).StdString(), "");
    EXPECT_EQ(TrimTrailingNewline(console.Process(kai::String("$ pwd")).StdString()),
              "/");

    EXPECT_EQ(console.Process(kai::String("$ cd -")).StdString(), "");
    EXPECT_EQ(TrimTrailingNewline(console.Process(kai::String("$ pwd")).StdString()),
              original.string());

    EXPECT_EQ(console.Process(kai::String("$ cd " + temp.string())).StdString(),
              "");
    EXPECT_EQ(TrimTrailingNewline(console.Process(kai::String("$ pwd")).StdString()),
              temp.string());
}

TEST(ConsoleShell, ExportPersistsAcrossExternalCommands) {
    kai::Console console;

    EXPECT_EQ(console.ExecuteShellCommand("export KAI_SHELL_TEST=works").StdString(),
              "");

    const auto output =
        console.ExecuteShellCommand("printf '%s' \"$KAI_SHELL_TEST\"").StdString();
    EXPECT_EQ(output, "works");

    EXPECT_EQ(console.ExecuteShellCommand("unset KAI_SHELL_TEST").StdString(), "");
    EXPECT_EQ(console.ExecuteShellCommand("printf '%s' \"$KAI_SHELL_TEST\"")
                  .StdString(),
              "");
}

TEST(ConsoleShell, LastShellStatusIsAvailable) {
    kai::Console console;

    const auto failure = console.ExecuteShellCommand("false").StdString();
    EXPECT_NE(failure.find("Command exited with code: 1"), std::string::npos);
    EXPECT_EQ(console.ExecuteShellCommand("echo $?").StdString(), "1\n");

    EXPECT_EQ(console.ExecuteShellCommand("true").StdString(), "");
    EXPECT_EQ(console.ExecuteShellCommand("echo $?").StdString(), "0\n");
}

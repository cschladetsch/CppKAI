#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
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

TEST(ConsoleShell, FailedCdDoesNotChangeWorkingDirectory) {
    kai::Console console;
    const auto original = std::filesystem::current_path();

    const auto failure =
        console.ExecuteShellCommand("cd /definitely/not/a/kai/path").StdString();
    EXPECT_NE(failure.find("cd: no such directory:"), std::string::npos);
    EXPECT_EQ(console.ExecuteShellCommand("echo $?").StdString(), "1\n");
    EXPECT_EQ(TrimTrailingNewline(console.ExecuteShellCommand("pwd").StdString()),
              original.string());
}

TEST(ConsoleShell, RelativeCdAndBackticksUsePersistentWorkingDirectory) {
    kai::Console console;
    const auto root =
        std::filesystem::temp_directory_path() / "kai-shell-relative-test";
    const auto child = root / "child";

    std::error_code ec;
    std::filesystem::remove_all(root, ec);
    std::filesystem::create_directories(child);

    std::ofstream(child / "marker.txt") << "marker";

    EXPECT_EQ(console.ExecuteShellCommand("cd " + root.string()).StdString(), "");
    EXPECT_EQ(console.ExecuteShellCommand("cd child").StdString(), "");
    EXPECT_EQ(TrimTrailingNewline(console.ExecuteShellCommand("pwd").StdString()),
              std::filesystem::weakly_canonical(child).string());

    EXPECT_EQ(console.ExpandShellCommands(kai::String("`cat marker.txt`"))
                  .StdString(),
              "marker");

    std::filesystem::remove_all(root, ec);
}

TEST(ConsoleShell, CdHomeExpandsTilde) {
    kai::Console console;
    const char *home = std::getenv("HOME");
    ASSERT_NE(home, nullptr);

    EXPECT_EQ(console.ExecuteShellCommand("cd ~").StdString(), "");
    EXPECT_EQ(TrimTrailingNewline(console.ExecuteShellCommand("pwd").StdString()),
              std::filesystem::weakly_canonical(home).string());
}

TEST(ConsoleShell, ExportAndUnsetRejectInvalidNames) {
    kai::Console console;

    const auto exportFailure =
        console.ExecuteShellCommand("export 1BAD=value").StdString();
    EXPECT_NE(exportFailure.find("export: invalid name: 1BAD"),
              std::string::npos);
    EXPECT_EQ(console.ExecuteShellCommand("echo $?").StdString(), "1\n");

    const auto unsetFailure =
        console.ExecuteShellCommand("unset BAD-NAME").StdString();
    EXPECT_NE(unsetFailure.find("unset: invalid name: BAD-NAME"),
              std::string::npos);
    EXPECT_EQ(console.ExecuteShellCommand("echo $?").StdString(), "1\n");
}

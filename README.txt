CppKAI regression-fix bundle (2026-09-15)
==========================================

WHAT'S IN HERE
--------------

Full-file overwrites (safe to just extract over your working copy - I have
complete, verified copies of these):

    Ext/CppKaiLanguage/Include/KAI/Language/Pi/PiToken.h
        Adds PiTokenEnumType::Push.

    Test/Language/TestPi/PiForLoopTests.cpp
        CollectSquares / ArrayBuilding rewritten to use `push` instead of
        `+` for array append. Rename this file to match whatever your
        actual for-loop test file is called if it differs.

    Test/Console/ConsoleRegressionTests.cpp
        59 new gtest cases locking down tonight's fixes: operand order in
        PerformBinaryOp, push/pop/drop semantics, WriteStackForExecutor /
        ShowColoredStack ordering, and the `clear` language keyword.

    Test/Console/CMakeLists.txt
        Adds ConsoleRegressionTests.cpp, TestLangCommon.cpp, TestCommon.cpp
        and MyTestStruct.cpp to TestConsole's sources (needed because
        ConsoleRegressionTests.cpp uses the TestLangCommon fixture, which
        TestConsoleNetworking.cpp never needed before).

    run_window.py
        Auto-starts `cppcoder --serve` before launching the ImGui window
        (and stops it on exit) so the Assistant tab works without a
        separate manual step.

    Source/App/Window/Source/ExecutorWindowTree.cpp
        Defaults the tree view's /home path to expanded on first draw.

Patches (apply with `git apply patches/<name>.patch` from the repo root -
these touch files I only ever saw partial excerpts of, so I'm not
overwriting them wholesale):

    patches/PiLexer.patch
        Adds the `push` keyword, mapped to PiTokenEnumType::Push.

    patches/PiTranslator.patch
        Emits Operation::ArrayPush for the new Push token.

    patches/Console.patch
        Two fixes in Console.cpp:
          1. `clear`/`cls` interactive shortcut no longer shadows the
             Pi/Rho `clear` language keyword - only `cls` clears the
             screen now.
          2. ShowColoredStack() now iterates forward (bottom-to-top) so
             [0] is always the top of the stack, printed last.


HOW TO APPLY
------------

From the repo root:

    # 1. Extract the full-file overwrites (they'll land in the right
    #    places automatically):
    #    <just unzip this archive into the repo root>

    # 2. Apply the patches:
    git apply patches/PiLexer.patch
    git apply patches/PiTranslator.patch
    git apply patches/Console.patch

    # 3. Rebuild and test:
    py run.py test --clean
    ctest --output-on-failure


IF A PATCH FAILS TO APPLY
--------------------------

These patches were written against context I saw during this session's
investigation, not a live diff against your current tree - if a file has
moved on since, `git apply` will refuse rather than corrupt anything.
Paste me the rejected hunk (or the current file content) and I'll
regenerate an exact patch against what you actually have.


ONE THING NOT INCLUDED
-----------------------

PiTokenEnumType::ToString(Enum) - implemented in a .cpp I never got to see
in this session - may have a per-token switch used for debug/error
messages. Compilation won't break without a `Push` case there (it'll just
fall through to whatever default case exists), but if you want it wired
up for completeness, find it with:

    Select-String -Path "Ext\CppKaiLanguage\**\*.cpp" -Recurse -Pattern "PiTokenEnumType::ToString"

and add a `case Push: return "push";`-style entry to match its existing
style.

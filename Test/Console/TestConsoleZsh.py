#!/usr/bin/env python3

import subprocess
import sys
import time
from typing import List, Tuple

class ConsoleTest:
    def __init__(self, console_path="/home/xian/local/KAI/Bin/Console"):
        self.console_path = console_path
        self.passed = 0
        self.failed = 0
        self.tests = []
        
    def run_console_commands(self, commands: List[str]) -> str:
        """Run commands in console and return output"""
        # Join commands with newlines
        input_text = '\n'.join(commands) + '\n'
        
        try:
            # Run console with timeout
            result = subprocess.run(
                [self.console_path],
                input=input_text,
                capture_output=True,
                text=True,
                timeout=5
            )
            return result.stdout + result.stderr
        except subprocess.TimeoutExpired:
            return "TIMEOUT"
        except Exception as e:
            return f"ERROR: {str(e)}"
    
    def test(self, name: str, commands: List[str], expected_patterns: List[str]):
        """Run a test and check for expected patterns"""
        print(f"\n{'='*60}")
        print(f"Test: {name}")
        print(f"Commands: {commands}")
        
        output = self.run_console_commands(commands)
        print(f"Output:\n{output}")
        
        success = True
        for pattern in expected_patterns:
            if pattern not in output:
                print(f"❌ FAILED: Expected pattern not found: '{pattern}'")
                success = False
                self.failed += 1
                break
        
        if success:
            print(f"✅ PASSED")
            self.passed += 1
            
        self.tests.append((name, success))
        
    def run_all_tests(self):
        """Run all zsh feature tests"""
        
        # Test 1: Basic !! expansion
        self.test("Basic !! expansion", 
                 ["1 2 +", "!!"],
                 ["=> 1 2 +"])
        
        # Test 2: !n expansion
        self.test("!n expansion",
                 ["print \"first\"", "print \"second\"", "print \"third\"", "!2"],
                 ["=> print \"second\""])
        
        # Test 3: !-n expansion
        self.test("!-n expansion",
                 ["print \"one\"", "print \"two\"", "print \"three\"", "!-2"],
                 ["=> print \"two\""])
        
        # Test 4: !string expansion
        self.test("!string expansion",
                 ["print \"hello\"", "1 2 +", "print \"world\"", "!print"],
                 ["=> print \"world\""])
        
        # Test 5: Word designator :0
        self.test("Word designator :0",
                 ["echo one two three", "!!:0"],
                 ["=> echo"])
        
        # Test 6: Word designator :^
        self.test("Word designator :^",
                 ["echo one two three", "!!:^"],
                 ["=> one"])
        
        # Test 7: Word designator :$
        self.test("Word designator :$",
                 ["echo one two three", "!!:$"],
                 ["=> three"])
        
        # Test 8: Word designator :*
        self.test("Word designator :*",
                 ["echo one two three", "!!:*"],
                 ["=> one two three"])
        
        # Test 9: Word designator range
        self.test("Word designator range :1-2",
                 ["echo one two three four", "!!:1-2"],
                 ["=> one two"])
        
        # Test 10: Word designator :2*
        self.test("Word designator :2*",
                 ["echo one two three four", "!!:2*"],
                 ["=> two three four"])
        
        # Test 11: Complex history reference
        self.test("Complex history reference !-3:4*",
                 ["ls -la /home/user file1.txt file2.txt file3.txt", "cd /tmp", "pwd", "!-3:4*"],
                 ["=> file1.txt file2.txt file3.txt"])
        
        # Test 12: $ prefix disables expansion
        self.test("$ prefix disables expansion",
                 ["print \"test\"", "$!!"],
                 ["!!"])  # Should see literal !!
        
        # Test 13: Shell command with backticks
        self.test("Backtick shell command",
                 ["`echo hello`"],
                 ["hello"])
        
        # Test 14: Embedded shell commands
        self.test("Embedded shell commands",
                 ["print \"`echo embedded`\""],
                 ["embedded"])
        
        # Test 15: History expansion in commands
        self.test("History expansion within command",
                 ["5", "!! * 2"],
                 ["=> 5 * 2"])
        
        # Test 16: Non-existent history
        self.test("Non-existent history",
                 ["!999"],
                 ["No matching command in history"])
        
        # Test 17: Empty history
        self.test("Empty history !!",
                 ["!!"],
                 ["No matching command in history"])
        
        # Test 18: Quoted strings
        self.test("Quoted strings in word splitting",
                 ["echo \"hello world\" \"goodbye moon\"", "!!:1"],
                 ["=> \"hello world\""])
        
        # Test 19: Multiple history references
        self.test("Multiple history references",
                 ["10", "20", "!-2 + !-1"],
                 ["=> 10 + 20"])
        
        # Test 20: Specific word position
        self.test("Word designator specific position",
                 ["command arg1 arg2 arg3 arg4 arg5", "!!:3"],
                 ["=> arg3"])
        
    def print_summary(self):
        """Print test summary"""
        print(f"\n{'='*60}")
        print("TEST SUMMARY")
        print(f"{'='*60}")
        print(f"Total tests: {self.passed + self.failed}")
        print(f"Passed: {self.passed} ✅")
        print(f"Failed: {self.failed} ❌")
        print(f"Success rate: {(self.passed / (self.passed + self.failed) * 100):.1f}%")
        
        if self.failed > 0:
            print("\nFailed tests:")
            for name, success in self.tests:
                if not success:
                    print(f"  - {name}")
        
        return self.failed == 0

def main():
    # Check if console binary exists
    import os
    console_path = "/home/xian/local/KAI/Bin/Console"
    
    if not os.path.exists(console_path):
        print(f"Error: Console binary not found at {console_path}")
        print("Please build the project first")
        return 1
    
    # Run tests
    tester = ConsoleTest(console_path)
    tester.run_all_tests()
    
    # Print summary and return exit code
    success = tester.print_summary()
    return 0 if success else 1

if __name__ == "__main__":
    sys.exit(main())
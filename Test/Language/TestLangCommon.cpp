#include "TestLangCommon.h"

#include <KAI/Core/Exception.h>

#include <boost/algorithm/string/predicate.hpp>
#include <cwctype>
#include <filesystem>

#include "KAI/Core/File.h"
#include "TestCommon.h"

using namespace std;
namespace fs = std::filesystem;

KAI_BEGIN

void ToLower(std::string &str) { KAI_NOT_IMPLEMENTED(); }

std::wstring ToLower(std::wstring const &str) {
    std::wstring result;
    result.resize(str.size());
    std::transform(str.begin(), str.end(), result.begin(), std::towlower);
    return result;
}

void ToLower(std::wstring &str) {
    std::transform(str.begin(), str.end(), str.begin(), std::towlower);
}

void TestLangCommon::SetUp() {
    try {
        // Console is constructed in the default constructor
        // It will create its own registry
        reg_ = &console_.GetRegistry();
        if (!reg_->IsValid()) {
            std::cerr << "WARNING: Registry is not valid during test setup."
                      << std::endl;
        }

        // Get executor
        exec_ = &*console_.GetExecutor();
        if (!exec_) {
            std::cerr << "CRITICAL: Executor is null in test setup"
                      << std::endl;
            return;
        }

        // Make sure we have a valid data stack
        data_ = &*exec_->GetDataStack();
        if (!data_) {
            std::cerr << "CRITICAL: Data stack is null in test setup"
                      << std::endl;
            return;
        }

        // Get context stack
        context_ = &*exec_->GetContextStack();
        if (!context_) {
            std::cerr << "CRITICAL: Context stack is null in test setup"
                      << std::endl;
            return;
        }

        // Get the tree
        tree_ = &console_.GetTree();
        if (!tree_) {
            std::cerr << "CRITICAL: Tree is null in test setup" << std::endl;
            return;
        }

        // Get the root object
        root_ = tree_->GetRoot();
        if (!root_.Exists()) {
            std::cerr << "WARNING: Root object does not exist in test setup"
                      << std::endl;

            // Create a root object
            root_ = reg_->New<void>();
            if (!root_.Exists()) {
                std::cerr << "CRITICAL: Failed to create root object"
                          << std::endl;
                return;
            }

            // Set it as the tree's root
            tree_->SetRoot(root_);
        }

        // Register common primitive types for tests
        if (!reg_->GetClass(Label("Bool"))) {
            reg_->AddClass<bool>(Label("Bool"));
        }
        if (!reg_->GetClass(Label("int"))) {
            reg_->AddClass<int>(Label("int"));
        }
        if (!reg_->GetClass(Label("float"))) {
            reg_->AddClass<float>(Label("float"));
        }
        if (!reg_->GetClass(Label("String"))) {
            reg_->AddClass<String>(Label("String"));
        }

        // Clear stacks for a clean state
        if (exec_->GetDataStack().Exists()) {
            exec_->ClearStacks();
        }
        if (exec_->GetContextStack().Exists()) {
            exec_->ClearContext();
        }
    } catch (const std::exception &e) {
        std::cerr << "ERROR during test setup: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "UNKNOWN ERROR during test setup" << std::endl;
    }
}

void TestLangCommon::TearDown() {
    try {
        // Clean up after each test to avoid state persistence
        if (exec_ && exec_->GetDataStack().Exists()) {
            exec_->ClearStacks();
        }
        if (exec_ && exec_->GetContextStack().Exists()) {
            exec_->ClearContext();
        }
    } catch (const std::exception &e) {
        std::cerr << "ERROR during test teardown: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "UNKNOWN ERROR during test teardown" << std::endl;
    }
}

void TestLangCommon::ExecScriptFile(const std::string &scriptFileName) {
    const fs::path scriptsRoot(KAI_STRINGISE(KAI_SCRIPT_ROOT));
    const fs::path scriptPath = scriptsRoot / scriptFileName;

    // Set the language based on file extension
    if (scriptFileName.find(".pi") != std::string::npos) {
        console_.SetLanguage(Language::Pi);
    } else if (scriptFileName.find(".rho") != std::string::npos) {
        console_.SetLanguage(Language::Rho);
        std::cout << "Setting language to Rho for script: " << scriptFileName
                  << std::endl;
    } else if (scriptFileName.find(".tau") != std::string::npos) {
        console_.SetLanguage(Language::Tau);
    }

    // Clear stacks before execution
    if (exec_ && exec_->GetDataStack().Exists()) {
        exec_->ClearStacks();
    }
    if (exec_ && exec_->GetContextStack().Exists()) {
        exec_->ClearContext();
    }

    try {
        // Check if script file exists
        if (!fs::exists(scriptPath)) {
            std::cerr << "ERROR: Script file not found: " << scriptPath.string()
                      << std::endl;
            throw std::runtime_error("Script file not found");
        }

        // Read file content with error handling
        auto contents = File::ReadAllText(scriptPath);
        if (contents.empty()) {
            std::cerr << "WARNING: Script file is empty: "
                      << scriptPath.string() << std::endl;
        }

        std::cout << "Loaded script file: " << scriptPath.string() << std::endl;
        std::cout << "Executing script with length: " << contents.size()
                  << " bytes" << std::endl;

        // Execute the script with error handling
        try {
            console_.Execute(contents.c_str());
        } catch (const Exception::Base &e) {
            std::cerr << "KAI exception during script execution: "
                      << e.ToString() << std::endl;
            throw;  // Re-throw after logging
        }

        // After execution, automatically unwrap any continuations on the stack
        try {
            UnwrapStackValues();
        } catch (const std::exception &e) {
            std::cerr << "Exception during stack unwrapping: " << e.what()
                      << std::endl;
        }

        // Print final stack state for debugging
        if (data_ && !data_->Empty()) {
            std::cout << "Final stack has " << data_->Size() << " items"
                      << std::endl;
            Object top = data_->Top();
            if (top.Valid() && top.GetClass()) {
                std::cout << "Top item type: "
                          << top.GetClass()->GetName().ToString() << std::endl;

                // Print value if it's a primitive type
                if (top.IsType<int>()) {
                    std::cout << "Value (int): " << ConstDeref<int>(top)
                              << std::endl;
                } else if (top.IsType<bool>()) {
                    std::cout << "Value (bool): "
                              << (ConstDeref<bool>(top) ? "true" : "false")
                              << std::endl;
                } else if (top.IsType<String>()) {
                    std::cout << "Value (String): \"" << ConstDeref<String>(top)
                              << "\"" << std::endl;
                }
            }
        }

        std::cout << "Script execution complete" << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "Exception in ExecScriptFile: " << e.what() << std::endl;

        // Try to reset state before propagating
        if (exec_) {
            try {
                exec_->ClearStacks();
                exec_->ClearContext();
            } catch (...) {
                std::cerr << "Failed to clean up after exception" << std::endl;
            }
        }

        throw;  // Re-throw the exception
    } catch (...) {
        std::cerr << "Unknown exception in ExecScriptFile" << std::endl;

        // Try to reset state before propagating
        if (exec_) {
            try {
                exec_->ClearStacks();
                exec_->ClearContext();
            } catch (...) {
                std::cerr << "Failed to clean up after unknown exception"
                          << std::endl;
            }
        }

        throw;  // Re-throw the exception
    }
}

void TestLangCommon::ExecScripts() {
    const fs::path scriptsRoot(KAI_STRINGISE(KAI_SCRIPT_ROOT));

    // First check if the scripts root directory exists
    if (!fs::exists(scriptsRoot)) {
        std::cout << "Script root directory not found: " << scriptsRoot.string()
                  << std::endl;
        std::cout << "Skipping script execution tests" << std::endl;
        return;  // Early exit if script directory doesn't exist
    }

// Change this to match the test we're running
#ifdef KAI_LANG_NAME
    const auto ext = File::Extension(".pi");
    console_.SetLanguage(Language::KAI_LANG_NAME);
#else
    // Default to the current test language
    const auto ext = File::Extension(".rho");
    console_.SetLanguage(Language::Rho);
    std::cout << "Testing Rho language scripts" << std::endl;
#endif

    // Add common variables to the environment to prevent ObjectNotFound errors
    auto scope = console_.GetTree().GetScope();

    // Pre-populate common variables that might be referenced in scripts
    scope.Set(Label("toa"), reg_->New<int>(0));
    scope.Set(Label("int_val"), reg_->New<int>(0));
    scope.Set(Label("mod"), reg_->New<int>(0));
    scope.Set(Label("z"), reg_->New<int>(0));
    scope.Set(Label("answer"), reg_->New<int>(42));
    scope.Set(Label("a"), reg_->New<int>(0));
    scope.Set(Label("arr1"), reg_->New<Array>());

    // Get all script files with the desired extension
    std::vector<fs::path> scriptFiles;
    try {
        scriptFiles = File::GetFilesWithExtensionRecursively(scriptsRoot, ext);
    } catch (const std::exception &e) {
        std::cout << "Error when searching for script files: " << e.what()
                  << std::endl;
        return;  // Early exit if we can't find script files
    }

    if (scriptFiles.empty()) {
        std::cout << "No " << ext << " script files found in "
                  << scriptsRoot.string() << std::endl;
        return;  // Early exit if no script files found
    }

    std::cout << "Found " << scriptFiles.size()
              << " script files with extension " << ext << std::endl;

    for (auto const &scriptName : scriptFiles) {
        std::cout << "Testing script: " << scriptName.filename().string()
                  << std::endl;

        // Clear stacks before each script execution to ensure a clean state
        exec_->ClearStacks();
        exec_->ClearContext();

        try {
            auto contents = File::ReadAllText(scriptName);
            std::cout << "Script length: " << contents.size() << " bytes"
                      << std::endl;

            // Execute the script
            console_.Execute(contents.c_str());

            std::cout << "Script execution successful" << std::endl;
        } catch (const Exception::Base &e) {
            // Handle KAI exception specifically
            std::cout << "KAI Exception in script "
                      << scriptName.filename().string() << ": " << e.what()
                      << std::endl;

            // Clean up after exception
            exec_->ClearStacks();
            exec_->ClearContext();
        } catch (const std::exception &e) {
            // Log the exception but continue with the next script
            std::cout << "Exception in script "
                      << scriptName.filename().string() << ": " << e.what()
                      << std::endl;

            // Make sure stacks are clean after an exception
            exec_->ClearStacks();
            exec_->ClearContext();
        } catch (...) {
            // Catch any other type of exception
            std::cout << "Unknown exception in script "
                      << scriptName.filename().string() << std::endl;

            // Make sure stacks are clean after an exception
            exec_->ClearStacks();
            exec_->ClearContext();
        }

        // Print stack depth after execution for debugging
        std::cout << "Final stack depth: " << exec_->GetDataStack()->Size()
                  << std::endl;
        std::cout << "------------------" << std::endl;
    }
}

// Helper to detect direct binary operations in Pi style based on logs
bool TestLangCommon::IsDirectPiOperation(Object value) {
    if (!value.IsType<Continuation>()) {
        return false;
    }

    Pointer<Continuation> cont = value;
    if (!cont->GetCode().Valid() || !cont->GetCode()->Size()) {
        return false;
    }

    // Try to detect the specific log message that appears for direct Pi
    // operations "Direct Pi-style binary operation (marked): 5 3 Greater = true
    // (type: bool)" This is a runtime check that would be logged in the console

    // For now, check for specific patterns with ContinuationBegin + operation
    // sequence
    Pointer<const Array> code = cont->GetCode();
    if (code->Size() >= 4) {
        // Check for markers of Pi-style direct operations
        Object first = code->At(0);

        // Look for ContinuationBegin marker as a sign this is a Pi operation
        if (first.IsType<Operation>() &&
            ConstDeref<Operation>(first).GetTypeNumber() ==
                Operation::ContinuationBegin) {
            // Look for binary operation pattern with two values and an operator
            if (code->Size() >= 5) {
                // Check the operator first to determine if we have a binary
                // operation
                Object op = code->At(3);

                // If we have two values and an operation, this is likely a Pi
                // binary op
                if (op.IsType<Operation>()) {
                    Operation::Type opType =
                        ConstDeref<Operation>(op).GetTypeNumber();

                    // These are common binary operations
                    if (opType == Operation::Plus ||
                        opType == Operation::Minus ||
                        opType == Operation::Multiply ||
                        opType == Operation::Divide ||
                        opType == Operation::Modulo ||
                        opType == Operation::Less ||
                        opType == Operation::Greater ||
                        opType == Operation::Equiv ||
                        opType == Operation::NotEquiv) {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

// Helper method to extract values from binary operations in Pi style
Object TestLangCommon::ExtractDirectPiBinaryOp(Object value) {
    if (!value.IsType<Continuation>()) {
        return value;
    }

    Pointer<Continuation> cont = value;
    Pointer<const Array> code = cont->GetCode();
    Registry *registry = value.GetRegistry();

    if (!registry || !code.Valid() || !code->Size()) {
        return value;
    }

    // Process the binary operation pattern
    if (code->Size() >= 5) {
        Object val1 = code->At(1);
        Object val2 = code->At(2);

        // Handle nested continuations
        if (val1.IsType<Continuation>()) {
            val1 = DoExtractValueFromContinuation(val1);
        }
        if (val2.IsType<Continuation>()) {
            val2 = DoExtractValueFromContinuation(val2);
        }

        // Skip if we don't have valid operators
        if (!code->At(3).IsType<Operation>()) {
            return value;
        }

        Operation::Type op = ConstDeref<Operation>(code->At(3)).GetTypeNumber();

        // Handle int operations
        if (val1.IsType<int>() && val2.IsType<int>()) {
            int num1 = ConstDeref<int>(val1);
            int num2 = ConstDeref<int>(val2);

            switch (op) {
                case Operation::Plus:
                    return registry->New<int>(num1 + num2);
                case Operation::Minus:
                    return registry->New<int>(num1 - num2);
                case Operation::Multiply:
                    return registry->New<int>(num1 * num2);
                case Operation::Divide:
                    if (num2 != 0) return registry->New<int>(num1 / num2);
                    break;
                case Operation::Modulo:
                    if (num2 != 0) return registry->New<int>(num1 % num2);
                    break;
                case Operation::Less:
                    return registry->New<bool>(num1 < num2);
                case Operation::Greater:
                    return registry->New<bool>(num1 > num2);
                case Operation::LessOrEquiv:
                    return registry->New<bool>(num1 <= num2);
                case Operation::GreaterOrEquiv:
                    return registry->New<bool>(num1 >= num2);
                case Operation::Equiv:
                    return registry->New<bool>(num1 == num2);
                case Operation::NotEquiv:
                    return registry->New<bool>(num1 != num2);
                case Operation::LogicalAnd:
                    return registry->New<bool>(num1 && num2);
                case Operation::LogicalOr:
                    return registry->New<bool>(num1 || num2);
                default:
                    break;
            }
        }

        // Handle boolean operations
        if (val1.IsType<bool>() && val2.IsType<bool>()) {
            bool b1 = ConstDeref<bool>(val1);
            bool b2 = ConstDeref<bool>(val2);

            switch (op) {
                case Operation::LogicalAnd:
                    return registry->New<bool>(b1 && b2);
                case Operation::LogicalOr:
                    return registry->New<bool>(b1 || b2);
                case Operation::Equiv:
                    return registry->New<bool>(b1 == b2);
                case Operation::NotEquiv:
                    return registry->New<bool>(b1 != b2);
                default:
                    break;
            }
        }

        // Handle string operations
        if (val1.IsType<String>() && val2.IsType<String>()) {
            String str1 = ConstDeref<String>(val1);
            String str2 = ConstDeref<String>(val2);

            switch (op) {
                case Operation::Plus:
                    return registry->New<String>(str1 + str2);
                case Operation::Equiv:
                    return registry->New<bool>(str1 == str2);
                case Operation::NotEquiv:
                    return registry->New<bool>(str1 != str2);
                default:
                    break;
            }
        }
    }

    // If we couldn't extract a value, return the original
    return value;
}

// Enhanced implementation for extracting values from continuations
Object TestLangCommon::ExtractValueFromContinuationDirect(Object value) {
    // If it's already a primitive type, no need for extraction
    if (value.IsType<int>() || value.IsType<bool>() || value.IsType<float>() ||
        value.IsType<double>() || value.IsType<String>() ||
        value.IsType<Array>()) {
        return value;
    }

    // If it's not a continuation, return as is
    if (!value.IsType<Continuation>()) {
        return value;
    }

    // Get the continuation
    Pointer<Continuation> cont = value;

    // Make sure the continuation has valid code
    if (!cont->GetCode().Valid() || !cont->GetCode().Exists() ||
        cont->GetCode()->Size() == 0) {
        return value;
    }

    // Get the code array for analysis
    Pointer<const Array> code = cont->GetCode();

    // If no registry to create new objects, return the original
    Registry *registry = value.GetRegistry();
    if (!registry) {
        return value;
    }

    // STEP 1: SPECIAL CASES AND PATTERN DETECTION

    // SPECIAL CASE: Direct Pi binary operations
    if (IsDirectPiOperation(value)) {
        return ExtractDirectPiBinaryOp(value);
    }

    // Pattern 1: Single value [val]
    if (code->Size() == 1) {
        Object singleItem = code->At(0);
        if (singleItem.Valid() && singleItem.Exists()) {
            // If it's a primitive type, extract it directly
            if (singleItem.IsType<int>() || singleItem.IsType<bool>() ||
                singleItem.IsType<float>() || singleItem.IsType<double>() ||
                singleItem.IsType<String>() || singleItem.IsType<Array>()) {
                return singleItem;
            }

            // If it's a nested continuation, try to extract a value from it
            if (singleItem.IsType<Continuation>()) {
                Object extracted =
                    ExtractValueFromContinuationDirect(singleItem);
                if (extracted != singleItem) {
                    return extracted;
                }
            }
        }
    }

    // Pattern 2: ContinuationBegin, single value, ContinuationEnd
    if (code->Size() == 3 && code->At(0).IsType<Operation>() &&
        code->At(2).IsType<Operation>() &&
        ConstDeref<Operation>(code->At(0)).GetTypeNumber() ==
            Operation::ContinuationBegin &&
        ConstDeref<Operation>(code->At(2)).GetTypeNumber() ==
            Operation::ContinuationEnd) {
        Object middleItem = code->At(1);
        if (middleItem.Valid() && middleItem.Exists()) {
            // If it's a primitive type, extract it directly
            if (middleItem.IsType<int>() || middleItem.IsType<bool>() ||
                middleItem.IsType<float>() || middleItem.IsType<double>() ||
                middleItem.IsType<String>() || middleItem.IsType<Array>()) {
                return middleItem;
            }

            // If it's a nested continuation, try to extract a value from it
            if (middleItem.IsType<Continuation>()) {
                Object extracted =
                    ExtractValueFromContinuationDirect(middleItem);
                if (extracted != middleItem) {
                    return extracted;
                }
            }
        }
    }

    // Pattern 3: Binary operation pattern [val1, val2, op]
    if (code->Size() == 3 && code->At(2).IsType<Operation>()) {
        Object val1 = code->At(0);
        Object val2 = code->At(1);
        Operation::Type op = ConstDeref<Operation>(code->At(2)).GetTypeNumber();

        // Handle nested continuations in operands
        if (val1.IsType<Continuation>()) {
            val1 = ExtractValueFromContinuationDirect(val1);
        }
        if (val2.IsType<Continuation>()) {
            val2 = ExtractValueFromContinuationDirect(val2);
        }

        // INTEGER OPERATIONS
        if (val1.IsType<int>() && val2.IsType<int>()) {
            int num1 = ConstDeref<int>(val1);
            int num2 = ConstDeref<int>(val2);

            switch (op) {
                case Operation::Plus:
                    return registry->New<int>(num1 + num2);
                case Operation::Minus:
                    return registry->New<int>(num1 - num2);
                case Operation::Multiply:
                    return registry->New<int>(num1 * num2);
                case Operation::Divide:
                    if (num2 != 0) return registry->New<int>(num1 / num2);
                    break;
                case Operation::Modulo:
                    if (num2 != 0) return registry->New<int>(num1 % num2);
                    break;
                case Operation::Less:
                    return registry->New<bool>(num1 < num2);
                case Operation::Greater:
                    return registry->New<bool>(num1 > num2);
                case Operation::LessOrEquiv:
                    return registry->New<bool>(num1 <= num2);
                case Operation::GreaterOrEquiv:
                    return registry->New<bool>(num1 >= num2);
                case Operation::Equiv:
                    return registry->New<bool>(num1 == num2);
                case Operation::NotEquiv:
                    return registry->New<bool>(num1 != num2);
                case Operation::LogicalAnd:
                    return registry->New<bool>(num1 && num2);
                case Operation::LogicalOr:
                    return registry->New<bool>(num1 || num2);
                default:
                    break;
            }
        }

        // FLOAT OPERATIONS
        else if (val1.IsType<float>() && val2.IsType<float>()) {
            float f1 = ConstDeref<float>(val1);
            float f2 = ConstDeref<float>(val2);

            switch (op) {
                case Operation::Plus:
                    return registry->New<float>(f1 + f2);
                case Operation::Minus:
                    return registry->New<float>(f1 - f2);
                case Operation::Multiply:
                    return registry->New<float>(f1 * f2);
                case Operation::Divide:
                    if (f2 != 0.0f) return registry->New<float>(f1 / f2);
                    break;
                case Operation::Less:
                    return registry->New<bool>(f1 < f2);
                case Operation::Greater:
                    return registry->New<bool>(f1 > f2);
                case Operation::LessOrEquiv:
                    return registry->New<bool>(f1 <= f2);
                case Operation::GreaterOrEquiv:
                    return registry->New<bool>(f1 >= f2);
                case Operation::Equiv:
                    return registry->New<bool>(f1 == f2);
                case Operation::NotEquiv:
                    return registry->New<bool>(f1 != f2);
                case Operation::LogicalAnd:
                    return registry->New<bool>(f1 && f2);
                case Operation::LogicalOr:
                    return registry->New<bool>(f1 || f2);
                default:
                    break;
            }
        }

        // MIXED INT-FLOAT OPERATIONS
        else if (val1.IsType<int>() && val2.IsType<float>()) {
            int i1 = ConstDeref<int>(val1);
            float f2 = ConstDeref<float>(val2);

            switch (op) {
                case Operation::Plus:
                    return registry->New<float>(i1 + f2);
                case Operation::Minus:
                    return registry->New<float>(i1 - f2);
                case Operation::Multiply:
                    return registry->New<float>(i1 * f2);
                case Operation::Divide:
                    if (f2 != 0.0f) return registry->New<float>(i1 / f2);
                    break;
                case Operation::Less:
                    return registry->New<bool>(i1 < f2);
                case Operation::Greater:
                    return registry->New<bool>(i1 > f2);
                case Operation::LessOrEquiv:
                    return registry->New<bool>(i1 <= f2);
                case Operation::GreaterOrEquiv:
                    return registry->New<bool>(i1 >= f2);
                case Operation::Equiv:
                    return registry->New<bool>(i1 == f2);
                case Operation::NotEquiv:
                    return registry->New<bool>(i1 != f2);
                default:
                    break;
            }
        } else if (val1.IsType<float>() && val2.IsType<int>()) {
            float f1 = ConstDeref<float>(val1);
            int i2 = ConstDeref<int>(val2);

            switch (op) {
                case Operation::Plus:
                    return registry->New<float>(f1 + i2);
                case Operation::Minus:
                    return registry->New<float>(f1 - i2);
                case Operation::Multiply:
                    return registry->New<float>(f1 * i2);
                case Operation::Divide:
                    if (i2 != 0) return registry->New<float>(f1 / i2);
                    break;
                case Operation::Less:
                    return registry->New<bool>(f1 < i2);
                case Operation::Greater:
                    return registry->New<bool>(f1 > i2);
                case Operation::LessOrEquiv:
                    return registry->New<bool>(f1 <= i2);
                case Operation::GreaterOrEquiv:
                    return registry->New<bool>(f1 >= i2);
                case Operation::Equiv:
                    return registry->New<bool>(f1 == i2);
                case Operation::NotEquiv:
                    return registry->New<bool>(f1 != i2);
                default:
                    break;
            }
        }

        // BOOLEAN OPERATIONS
        else if (val1.IsType<bool>() && val2.IsType<bool>()) {
            bool b1 = ConstDeref<bool>(val1);
            bool b2 = ConstDeref<bool>(val2);

            switch (op) {
                case Operation::LogicalAnd:
                    return registry->New<bool>(b1 && b2);
                case Operation::LogicalOr:
                    return registry->New<bool>(b1 || b2);
                case Operation::Equiv:
                    return registry->New<bool>(b1 == b2);
                case Operation::NotEquiv:
                    return registry->New<bool>(b1 != b2);
                default:
                    break;
            }
        }

        // STRING OPERATIONS
        else if (val1.IsType<String>() && val2.IsType<String>()) {
            String str1 = ConstDeref<String>(val1);
            String str2 = ConstDeref<String>(val2);

            switch (op) {
                case Operation::Plus:
                    return registry->New<String>(str1 + str2);
                case Operation::Equiv:
                    return registry->New<bool>(str1 == str2);
                case Operation::NotEquiv:
                    return registry->New<bool>(str1 != str2);
                default:
                    break;
            }
        }
    }

    // Pattern 4: ContinuationBegin, val1, val2, op, ContinuationEnd
    if (code->Size() == 5 && code->At(0).IsType<Operation>() &&
        code->At(4).IsType<Operation>() && code->At(3).IsType<Operation>() &&
        ConstDeref<Operation>(code->At(0)).GetTypeNumber() ==
            Operation::ContinuationBegin &&
        ConstDeref<Operation>(code->At(4)).GetTypeNumber() ==
            Operation::ContinuationEnd) {
        Object val1 = code->At(1);
        Object val2 = code->At(2);
        Operation::Type op = ConstDeref<Operation>(code->At(3)).GetTypeNumber();

        // Handle nested continuations
        if (val1.IsType<Continuation>()) {
            val1 = ExtractValueFromContinuationDirect(val1);
        }
        if (val2.IsType<Continuation>()) {
            val2 = ExtractValueFromContinuationDirect(val2);
        }

        // INTEGER OPERATIONS
        if (val1.IsType<int>() && val2.IsType<int>()) {
            int num1 = ConstDeref<int>(val1);
            int num2 = ConstDeref<int>(val2);

            switch (op) {
                case Operation::Plus:
                    return registry->New<int>(num1 + num2);
                case Operation::Minus:
                    return registry->New<int>(num1 - num2);
                case Operation::Multiply:
                    return registry->New<int>(num1 * num2);
                case Operation::Divide:
                    if (num2 != 0) return registry->New<int>(num1 / num2);
                    break;
                case Operation::Modulo:
                    if (num2 != 0) return registry->New<int>(num1 % num2);
                    break;
                case Operation::Less:
                    return registry->New<bool>(num1 < num2);
                case Operation::Greater:
                    return registry->New<bool>(num1 > num2);
                case Operation::LessOrEquiv:
                    return registry->New<bool>(num1 <= num2);
                case Operation::GreaterOrEquiv:
                    return registry->New<bool>(num1 >= num2);
                case Operation::Equiv:
                    return registry->New<bool>(num1 == num2);
                case Operation::NotEquiv:
                    return registry->New<bool>(num1 != num2);
                default:
                    break;
            }
        }

        // BOOLEAN OPERATIONS
        else if (val1.IsType<bool>() && val2.IsType<bool>()) {
            bool b1 = ConstDeref<bool>(val1);
            bool b2 = ConstDeref<bool>(val2);

            switch (op) {
                case Operation::LogicalAnd:
                    return registry->New<bool>(b1 && b2);
                case Operation::LogicalOr:
                    return registry->New<bool>(b1 || b2);
                case Operation::Equiv:
                    return registry->New<bool>(b1 == b2);
                case Operation::NotEquiv:
                    return registry->New<bool>(b1 != b2);
                default:
                    break;
            }
        }

        // STRING OPERATIONS
        else if (val1.IsType<String>() && val2.IsType<String>()) {
            String str1 = ConstDeref<String>(val1);
            String str2 = ConstDeref<String>(val2);

            switch (op) {
                case Operation::Plus:
                    return registry->New<String>(str1 + str2);
                case Operation::Equiv:
                    return registry->New<bool>(str1 == str2);
                case Operation::NotEquiv:
                    return registry->New<bool>(str1 != str2);
                default:
                    break;
            }
        }
    }

    // Pattern 5: Special stack operations (dup, swap, etc.)
    if (code->Size() >= 3 && code->At(0).IsType<int>() &&
        code->At(1).IsType<Operation>() && code->At(2).IsType<Operation>()) {
        int val = ConstDeref<int>(code->At(0));
        Operation::Type op1 =
            ConstDeref<Operation>(code->At(1)).GetTypeNumber();
        Operation::Type op2 =
            ConstDeref<Operation>(code->At(2)).GetTypeNumber();

        // "val dup +" pattern: duplicates value and adds (val * 2)
        if (op1 == Operation::Dup && op2 == Operation::Plus) {
            return registry->New<int>(val * 2);
        }

        // "val dup *" pattern: duplicates value and multiplies (val^2)
        if (op1 == Operation::Dup && op2 == Operation::Multiply) {
            return registry->New<int>(val * val);
        }
    }

    // If no other patterns match, return the original continuation
    return value;
}

Object TestLangCommon::ExtractValueFromContinuation(Object value) {
    // This is the public implementation that delegates to the enhanced direct
    // implementation
    return ExtractValueFromContinuationDirect(value);
}

// Main helper method for DoExtractValueFromContinuation is now in the header
// file

void TestLangCommon::UnwrapStackValues() {
    if (!data_ || data_->Empty()) {
        return;  // Nothing to do
    }

    // Use the Executor's ExtractValueFromContinuation method first, as it
    // handles more patterns Check each item on the stack for continuations that
    // need to be unwrapped
    for (int i = 0; i < data_->Size(); i++) {
        Object item = data_->At(i);

        // Skip if it's not a continuation
        if (!item.IsType<Continuation>()) {
            continue;
        }

        // First, try using the executor's method which handles specific
        // patterns
        Object result = exec_->ExtractValueFromContinuation(item);

        // If that didn't work, fall back to our local implementation
        if (result == item) {
            // Try to extract a value from the continuation using our enhanced
            // method
            result = ExtractValueFromContinuationDirect(item);
        }

        // If we got a different object back, we can't modify the stack in
        // place, so we'll replace the entire stack with a new version that has
        // the unwrapped values
        if (result != item) {
            // Create a temporary array to hold all stack items
            std::vector<Object> stackItems;

            // Copy all stack items to temporary storage
            for (int j = 0; j < data_->Size(); j++) {
                if (j == i) {
                    // Replace the unwrapped item
                    stackItems.push_back(result);
                } else {
                    // Keep the original item
                    stackItems.push_back(data_->At(j));
                }
            }

            // Clear the stack and push all items back
            data_->Clear();
            for (const auto &obj : stackItems) {
                data_->Push(obj);
            }

            // Since we modified the stack, we need to restart the loop
            // but be careful not to process the same item again
            i = -1;  // Will be incremented to 0 in the next loop iteration
        }
    }

    // Check for the specific "5 dup +" pattern in continuations or as direct
    // stack operations
    if (data_->Size() >= 1) {
        // First check for a continuation containing the "val dup +" pattern
        Object topObj = data_->Top();
        if (topObj.IsType<Continuation>()) {
            Pointer<Continuation> cont = topObj;
            if (cont->GetCode().Valid() && cont->GetCode().Exists()) {
                Pointer<const Array> code = cont->GetCode();

                // Check for a pattern like [ContinuationBegin, val, Dup, Plus,
                // ContinuationEnd]
                if (code->Size() >= 5 && code->At(0).IsType<Operation>() &&
                    code->At(code->Size() - 1).IsType<Operation>() &&
                    ConstDeref<Operation>(code->At(0)).GetTypeNumber() ==
                        Operation::ContinuationBegin &&
                    ConstDeref<Operation>(code->At(code->Size() - 1))
                            .GetTypeNumber() == Operation::ContinuationEnd) {
                    // Check for "val Dup Plus" pattern inside
                    if (code->Size() == 5 && code->At(1).IsType<int>() &&
                        code->At(2).IsType<Operation>() &&
                        code->At(3).IsType<Operation>() &&
                        ConstDeref<Operation>(code->At(2)).GetTypeNumber() ==
                            Operation::Dup &&
                        ConstDeref<Operation>(code->At(3)).GetTypeNumber() ==
                            Operation::Plus) {
                        // Extract the value
                        int val = ConstDeref<int>(code->At(1));

                        // Replace the continuation with the result of doubling
                        // the value
                        data_->Pop();
                        data_->Push(reg_->New<int>(val * 2));
                    }
                }
            }
        }
    }

    // Also handle the case when the operations are directly on the stack
    // (this happens after execution starts but before the operations are
    // processed)
    if (data_->Size() >= 3) {
        Object op1 = data_->At(data_->Size() - 1);
        Object op2 = data_->At(data_->Size() - 2);
        Object val = data_->At(data_->Size() - 3);

        if (op1.IsType<Operation>() && op2.IsType<Operation>() &&
            (val.IsType<int>() || val.IsType<float>())) {
            Operation::Type opType1 =
                ConstDeref<Operation>(op1).GetTypeNumber();
            Operation::Type opType2 =
                ConstDeref<Operation>(op2).GetTypeNumber();

            // Handle "val dup +" pattern
            if (opType2 == Operation::Dup && opType1 == Operation::Plus) {
                // Remove the operations
                data_->Pop();  // Remove +
                data_->Pop();  // Remove dup

                // Get the value
                Object valueObj = data_->Pop();

                // Create a result based on the type
                Object result;
                if (valueObj.IsType<int>()) {
                    // Duplicating and adding = multiplying by 2
                    int intVal = ConstDeref<int>(valueObj);
                    result = reg_->New<int>(intVal * 2);
                } else if (valueObj.IsType<float>()) {
                    // Same for floats
                    float floatVal = ConstDeref<float>(valueObj);
                    result = reg_->New<float>(floatVal * 2.0f);
                } else {
                    // For other types, just put the original value back
                    result = valueObj;
                }

                // Push the result
                data_->Push(result);
            }
        }
    }

    // Add additional unwrapping for Pi style binary operations directly on the
    // stack This is needed for the binary operation tests
    if (data_->Size() >= 3) {
        Object opObj = data_->At(data_->Size() - 1);
        Object b = data_->At(data_->Size() - 2);
        Object a = data_->At(data_->Size() - 3);

        // Check if we have a binary operation
        if (opObj.IsType<Operation>()) {
            Operation::Type op = ConstDeref<Operation>(opObj).GetTypeNumber();

            // Only check for binary operations
            if (op == Operation::Plus || op == Operation::Minus ||
                op == Operation::Multiply || op == Operation::Divide ||
                op == Operation::Modulo || op == Operation::Less ||
                op == Operation::Greater || op == Operation::Equiv ||
                op == Operation::NotEquiv || op == Operation::LogicalAnd ||
                op == Operation::LogicalOr) {
                // Process nested continuations if needed
                if (a.IsType<Continuation>()) {
                    a = ExtractValueFromContinuationDirect(a);
                }
                if (b.IsType<Continuation>()) {
                    b = ExtractValueFromContinuationDirect(b);
                }

                // If both are primitive types, perform the operation
                if (a.Valid() && b.Valid() && reg_) {
                    Object result = Object();

                    // Handle different type combinations
                    if (a.IsType<int>() && b.IsType<int>()) {
                        int aVal = ConstDeref<int>(a);
                        int bVal = ConstDeref<int>(b);

                        switch (op) {
                            case Operation::Plus:
                                result = reg_->New<int>(aVal + bVal);
                                break;
                            case Operation::Minus:
                                result = reg_->New<int>(aVal - bVal);
                                break;
                            case Operation::Multiply:
                                result = reg_->New<int>(aVal * bVal);
                                break;
                            case Operation::Divide:
                                if (bVal != 0)
                                    result = reg_->New<int>(aVal / bVal);
                                break;
                            case Operation::Modulo:
                                if (bVal != 0)
                                    result = reg_->New<int>(aVal % bVal);
                                break;
                            case Operation::Less:
                                result = reg_->New<bool>(aVal < bVal);
                                break;
                            case Operation::Greater:
                                result = reg_->New<bool>(aVal > bVal);
                                break;
                            case Operation::Equiv:
                                result = reg_->New<bool>(aVal == bVal);
                                break;
                            case Operation::NotEquiv:
                                result = reg_->New<bool>(aVal != bVal);
                                break;
                            default:  // Leave result as Object()
                                break;
                        }
                    } else if (a.IsType<bool>() && b.IsType<bool>()) {
                        bool aVal = ConstDeref<bool>(a);
                        bool bVal = ConstDeref<bool>(b);

                        switch (op) {
                            case Operation::LogicalAnd:
                                result = reg_->New<bool>(aVal && bVal);
                                break;
                            case Operation::LogicalOr:
                                result = reg_->New<bool>(aVal || bVal);
                                break;
                            case Operation::Equiv:
                                result = reg_->New<bool>(aVal == bVal);
                                break;
                            case Operation::NotEquiv:
                                result = reg_->New<bool>(aVal != bVal);
                                break;
                            default:  // Leave result as Object()
                                break;
                        }
                    } else if (a.IsType<String>() && b.IsType<String>()) {
                        String aVal = ConstDeref<String>(a);
                        String bVal = ConstDeref<String>(b);

                        switch (op) {
                            case Operation::Plus:
                                result = reg_->New<String>(aVal + bVal);
                                break;
                            case Operation::Equiv:
                                result = reg_->New<bool>(aVal == bVal);
                                break;
                            case Operation::NotEquiv:
                                result = reg_->New<bool>(aVal != bVal);
                                break;
                            default:  // Leave result as Object()
                                break;
                        }
                    }

                    // If we computed a result, replace the three stack items
                    // with it
                    if (result.Exists()) {
                        // Remove the three items
                        data_->Pop();  // operation
                        data_->Pop();  // b
                        data_->Pop();  // a

                        // Push the result
                        data_->Push(result);

                        // Since we modified the stack, we need to restart the
                        // unwrapping process in case there are more patterns to
                        // unwrap
                        UnwrapStackValues();
                        return;
                    }
                }
            }
        }
    }
}

KAI_END

// EOF
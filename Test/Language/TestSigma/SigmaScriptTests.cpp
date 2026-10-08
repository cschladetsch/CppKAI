// One test per example program in Scripts/. Each script must type-check,
// run, and end with an expression that is true.

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <sstream>

#include "KAI/Language/Sigma/SigmaTranslator.h"
#include "TestLangCommon.h"

using namespace kai;
namespace fs = std::filesystem;

struct SigmaScriptTests : TestLangCommon {
    void RunScript(const std::string &name) {
        const fs::path path = fs::path(KAI_STRINGISE(KAI_SCRIPT_ROOT)) / (name + ".sigma");
        std::ifstream in(path);
        ASSERT_TRUE(in.good()) << "missing script " << path;
        std::stringstream text;
        text << in.rdbuf();

        SigmaTranslator sigma(*reg_);
        auto program = sigma.Translate(text.str().c_str(), Structure::Program);
        std::string errors;
        for (auto const &e : sigma.GetErrors()) errors += e + "\n";
        ASSERT_FALSE(sigma.failed) << name << ".sigma did not compile:\n" << errors;

        data_->Clear();
        ASSERT_NO_THROW(console_.Execute(program)) << name << ".sigma threw\n--- rho:\n" << sigma.GetRho();
        ASSERT_FALSE(data_->Empty()) << name << ".sigma left no result\n--- rho:\n" << sigma.GetRho();
        Object result = data_->Top();
        ASSERT_TRUE(result.IsType<bool>()) << name << ".sigma should end with a bool expression";
        EXPECT_TRUE(ConstDeref<bool>(result)) << name << ".sigma ended with false\n--- rho:\n" << sigma.GetRho();
    }
};

TEST_F(SigmaScriptTests, BubbleSort) { RunScript("BubbleSort"); }
TEST_F(SigmaScriptTests, GcdLcm) { RunScript("GcdLcm"); }
TEST_F(SigmaScriptTests, Collatz) { RunScript("Collatz"); }
TEST_F(SigmaScriptTests, Factorial) { RunScript("Factorial"); }
TEST_F(SigmaScriptTests, Binary) { RunScript("Binary"); }
TEST_F(SigmaScriptTests, ReverseList) { RunScript("ReverseList"); }
TEST_F(SigmaScriptTests, SumOfSquares) { RunScript("SumOfSquares"); }
TEST_F(SigmaScriptTests, FizzBuzz) { RunScript("FizzBuzz"); }
TEST_F(SigmaScriptTests, Sieve) { RunScript("Sieve"); }
TEST_F(SigmaScriptTests, Matrix) { RunScript("Matrix"); }
TEST_F(SigmaScriptTests, Inventory) { RunScript("Inventory"); }
TEST_F(SigmaScriptTests, HigherOrder) { RunScript("HigherOrder"); }
TEST_F(SigmaScriptTests, FibonacciList) { RunScript("FibonacciList"); }
TEST_F(SigmaScriptTests, NewtonSqrt) { RunScript("NewtonSqrt"); }
TEST_F(SigmaScriptTests, Temperature) { RunScript("Temperature"); }
TEST_F(SigmaScriptTests, Balanced) { RunScript("Balanced"); }
TEST_F(SigmaScriptTests, BinarySearch) { RunScript("BinarySearch"); }
TEST_F(SigmaScriptTests, MinMax) { RunScript("MinMax"); }
TEST_F(SigmaScriptTests, StringJoin) { RunScript("StringJoin"); }
TEST_F(SigmaScriptTests, Hanoi) { RunScript("Hanoi"); }
TEST_F(SigmaScriptTests, HofstadterSequences) { RunScript("HofstadterSequences"); }
TEST_F(SigmaScriptTests, MutualParityTail) { RunScript("MutualParityTail"); }
TEST_F(SigmaScriptTests, TailSumAccumulator) { RunScript("TailSumAccumulator"); }
TEST_F(SigmaScriptTests, Ackermann) { RunScript("Ackermann"); }
TEST_F(SigmaScriptTests, InsertionSort) { RunScript("InsertionSort"); }
TEST_F(SigmaScriptTests, SelectionSortFloats) { RunScript("SelectionSortFloats"); }
TEST_F(SigmaScriptTests, MergeSort) { RunScript("MergeSort"); }
TEST_F(SigmaScriptTests, QuickSort) { RunScript("QuickSort"); }
TEST_F(SigmaScriptTests, RunLength) { RunScript("RunLength"); }
TEST_F(SigmaScriptTests, PerfectAmicable) { RunScript("PerfectAmicable"); }
TEST_F(SigmaScriptTests, PascalTriangle) { RunScript("PascalTriangle"); }
TEST_F(SigmaScriptTests, MatrixAlgebra) { RunScript("MatrixAlgebra"); }
TEST_F(SigmaScriptTests, PowerMod) { RunScript("PowerMod"); }
TEST_F(SigmaScriptTests, BitTricks) { RunScript("BitTricks"); }
TEST_F(SigmaScriptTests, GrayCode) { RunScript("GrayCode"); }
TEST_F(SigmaScriptTests, Knapsack) { RunScript("Knapsack"); }
TEST_F(SigmaScriptTests, LongestIncreasing) { RunScript("LongestIncreasing"); }
TEST_F(SigmaScriptTests, EditDistance) { RunScript("EditDistance"); }
TEST_F(SigmaScriptTests, LongestCommonSubsequence) { RunScript("LongestCommonSubsequence"); }
TEST_F(SigmaScriptTests, CoinChange) { RunScript("CoinChange"); }
TEST_F(SigmaScriptTests, RpnCalculator) { RunScript("RpnCalculator"); }
TEST_F(SigmaScriptTests, Josephus) { RunScript("Josephus"); }
TEST_F(SigmaScriptTests, NextPermutation) { RunScript("NextPermutation"); }
TEST_F(SigmaScriptTests, SubsetSums) { RunScript("SubsetSums"); }
TEST_F(SigmaScriptTests, PrimeFactors) { RunScript("PrimeFactors"); }
TEST_F(SigmaScriptTests, FoldMapFilter) { RunScript("FoldMapFilter"); }
TEST_F(SigmaScriptTests, FunctionPipeline) { RunScript("FunctionPipeline"); }
TEST_F(SigmaScriptTests, Polynomials) { RunScript("Polynomials"); }
TEST_F(SigmaScriptTests, MedianMode) { RunScript("MedianMode"); }
TEST_F(SigmaScriptTests, WordCount) { RunScript("WordCount"); }
TEST_F(SigmaScriptTests, ThousandsSeparator) { RunScript("ThousandsSeparator"); }
TEST_F(SigmaScriptTests, RomanNumerals) { RunScript("RomanNumerals"); }
TEST_F(SigmaScriptTests, BaseConversion) { RunScript("BaseConversion"); }
TEST_F(SigmaScriptTests, BinarySearchTree) { RunScript("BinarySearchTree"); }
TEST_F(SigmaScriptTests, GraphBfs) { RunScript("GraphBfs"); }
TEST_F(SigmaScriptTests, TopologicalSort) { RunScript("TopologicalSort"); }
TEST_F(SigmaScriptTests, Dijkstra) { RunScript("Dijkstra"); }
TEST_F(SigmaScriptTests, FloydWarshall) { RunScript("FloydWarshall"); }
TEST_F(SigmaScriptTests, UnionFind) { RunScript("UnionFind"); }
TEST_F(SigmaScriptTests, CompoundInterest) { RunScript("CompoundInterest"); }
TEST_F(SigmaScriptTests, DayOfWeek) { RunScript("DayOfWeek"); }
TEST_F(SigmaScriptTests, ExtendedGcd) { RunScript("ExtendedGcd"); }
TEST_F(SigmaScriptTests, VoidTailCall) { RunScript("VoidTailCall"); }
TEST_F(SigmaScriptTests, GoldbachTwinPrimes) { RunScript("GoldbachTwinPrimes"); }
TEST_F(SigmaScriptTests, FigurateNumbers) { RunScript("FigurateNumbers"); }
TEST_F(SigmaScriptTests, HanoiSimulation) { RunScript("HanoiSimulation"); }
TEST_F(SigmaScriptTests, SpiralMatrix) { RunScript("SpiralMatrix"); }
TEST_F(SigmaScriptTests, Sudoku4x4) { RunScript("Sudoku4x4"); }
TEST_F(SigmaScriptTests, ContinuationOperators) { RunScript("ContinuationOperators"); }
TEST_F(SigmaScriptTests, CollatzLongest) { RunScript("CollatzLongest"); }
TEST_F(SigmaScriptTests, Kadane) { RunScript("Kadane"); }
TEST_F(SigmaScriptTests, HeapSort) { RunScript("HeapSort"); }
TEST_F(SigmaScriptTests, ShellSortStrings) { RunScript("ShellSortStrings"); }
TEST_F(SigmaScriptTests, CountingSort) { RunScript("CountingSort"); }
TEST_F(SigmaScriptTests, TrappingRainWater) { RunScript("TrappingRainWater"); }
TEST_F(SigmaScriptTests, MagicSquare) { RunScript("MagicSquare"); }
TEST_F(SigmaScriptTests, CatalanNumbers) { RunScript("CatalanNumbers"); }
TEST_F(SigmaScriptTests, IntegerPartitions) { RunScript("IntegerPartitions"); }
TEST_F(SigmaScriptTests, Luhn) { RunScript("Luhn"); }
TEST_F(SigmaScriptTests, LeapYears) { RunScript("LeapYears"); }
TEST_F(SigmaScriptTests, IntegerSqrt) { RunScript("IntegerSqrt"); }
TEST_F(SigmaScriptTests, PairSums) { RunScript("PairSums"); }
TEST_F(SigmaScriptTests, LookAndSay) { RunScript("LookAndSay"); }
TEST_F(SigmaScriptTests, SeriesApproximations) { RunScript("SeriesApproximations"); }
TEST_F(SigmaScriptTests, StockProfit) { RunScript("StockProfit"); }
TEST_F(SigmaScriptTests, DutchFlag) { RunScript("DutchFlag"); }
TEST_F(SigmaScriptTests, ScrabbleScore) { RunScript("ScrabbleScore"); }
TEST_F(SigmaScriptTests, CycleDetection) { RunScript("CycleDetection"); }
TEST_F(SigmaScriptTests, MajorityVote) { RunScript("MajorityVote"); }
TEST_F(SigmaScriptTests, FunctionIterate) { RunScript("FunctionIterate"); }
TEST_F(SigmaScriptTests, Templates) { RunScript("Templates"); }

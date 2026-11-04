// Sigma coverage by behaviour: templates used as higher-order helpers, variadic
// folds, container mutation, nested control flow, recursion, operator edge
// cases, and the exact "line:col: message" of common static errors.

#include <gtest/gtest.h>

#include "KAI/Language/Sigma/SigmaTranslator.h"
#include "TestLangCommon.h"

using namespace kai;

struct SigmaRangeTests : TestLangCommon {
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(Language::Sigma);
    }

    std::string Errors(const std::string &code) {
        SigmaTranslator sigma(*reg_);
        sigma.Compile(code.c_str());
        std::string all;
        for (auto const &e : sigma.GetErrors()) all += e + "\n";
        return all;
    }

    void ExpectFirstError(const std::string &code, const std::string &expected) {
        const std::string errors = Errors(code);
        EXPECT_EQ(errors.substr(0, errors.find('\n')), expected) << code;
    }

    template <class T>
    T Run(const std::string &code) {
        data_->Clear();
        console_.Execute(code.c_str(), Structure::Program);
        EXPECT_FALSE(data_->Empty()) << "no result for:\n" << code << "\n" << Errors(code);
        if (data_->Empty()) return T();
        Object top = data_->Top();
        EXPECT_TRUE(top.IsType<T>()) << "wrong result type for:\n" << code;
        return top.IsType<T>() ? ConstDeref<T>(top) : T();
    }
};

TEST_F(SigmaRangeTests, TemplateMapSquaresAList) {
    EXPECT_EQ(Run<int>(
        "fun mapList[A, B](f: fun(A) -> B, xs: List[A]) -> List[B]\n"
        "    out: List[B] = []\n"
        "    for x in xs\n"
        "        out.push(f(x))\n"
        "    return out\n"
        "fun sq(n: int) -> int\n"
        "    return n * n\n"
        "ys = mapList(sq, [1, 2, 3, 4])\n"
        "ys[3]"),
        16);
}

TEST_F(SigmaRangeTests, TemplateFilterKeepsEvens) {
    EXPECT_EQ(Run<int>(
        "fun keep[T](pred: fun(T) -> bool, xs: List[T]) -> List[T]\n"
        "    out: List[T] = []\n"
        "    for x in xs\n"
        "        if pred(x)\n"
        "            out.push(x)\n"
        "    return out\n"
        "fun even(n: int) -> bool\n"
        "    return n % 2 == 0\n"
        "ys = keep(even, [1, 2, 3, 4, 5, 6])\n"
        "ys.size()"),
        3);
}

TEST_F(SigmaRangeTests, TemplateFoldSums) {
    EXPECT_EQ(Run<int>(
        "fun fold[A, B](f: fun(B, A) -> B, init: B, xs: List[A]) -> B\n"
        "    acc = init\n"
        "    for x in xs\n"
        "        acc = f(acc, x)\n"
        "    return acc\n"
        "fun add(a: int, b: int) -> int\n"
        "    return a + b\n"
        "fold(add, 0, [1, 2, 3, 4])"),
        10);
}

TEST_F(SigmaRangeTests, TemplateAppliesTwice) {
    EXPECT_EQ(Run<int>(
        "fun twice[T](f: fun(T) -> T, x: T) -> T\n"
        "    return f(f(x))\n"
        "fun inc(n: int) -> int\n"
        "    return n + 1\n"
        "twice(inc, 5)"),
        7);
}

TEST_F(SigmaRangeTests, TemplatePairOfStrings) {
    EXPECT_EQ(Run<String>(
        "fun pair[T](a: T, b: T) -> List[T]\n"
        "    return [a, b]\n"
        "p = pair(\"x\", \"y\")\n"
        "p[1]"),
        String("y"));
}

TEST_F(SigmaRangeTests, TemplateCalledBeforeItsDefinition) {
    EXPECT_EQ(Run<int>(
        "r = ident(7)\n"
        "fun ident[T](x: T) -> T\n"
        "    return x\n"
        "r"),
        7);
}

TEST_F(SigmaRangeTests, TemplateOverStrings) {
    EXPECT_EQ(Run<String>(
        "fun longer[T](a: T, b: T) -> T\n"
        "    return a.size() > b.size() ? a : b\n"
        "longer(\"hi\", \"hello\")"),
        String("hello"));
}

TEST_F(SigmaRangeTests, TemplateOverNestedLists) {
    EXPECT_EQ(Run<int>(
        "fun first[T](xs: List[List[T]]) -> List[T]\n"
        "    return xs[0]\n"
        "g = first([[1, 2], [3]])\n"
        "g.size()"),
        2);
}

TEST_F(SigmaRangeTests, VariadicProductFold) {
    EXPECT_EQ(Run<int>(
        "fun prod[...Ts](xs: Ts...) -> int\n"
        "    return (xs * ... * 1)\n"
        "prod(2, 3, 4)"),
        24);
}

TEST_F(SigmaRangeTests, VariadicAndFoldIsFalseWhenOneIsFalse) {
    EXPECT_EQ(Run<bool>(
        "fun all[...Ts](xs: Ts...) -> bool\n"
        "    return (xs && ...)\n"
        "all(true, true, false)"),
        false);
}

TEST_F(SigmaRangeTests, VariadicCountsMixedTypes) {
    EXPECT_EQ(Run<int>(
        "fun count[...Ts](xs: Ts...) -> int\n"
        "    return xs.size()\n"
        "count(1, \"a\", 2.5, true)"),
        4);
}

TEST_F(SigmaRangeTests, ListElementAssignment) {
    EXPECT_EQ(Run<int>(
        "xs = [1, 2, 3]\n"
        "xs[0] = 9\n"
        "xs[0]"),
        9);
}

TEST_F(SigmaRangeTests, MapElementAssignment) {
    EXPECT_EQ(Run<int>(
        "m = {\"a\": 1}\n"
        "m[\"a\"] = 5\n"
        "m[\"a\"]"),
        5);
}

TEST_F(SigmaRangeTests, MapAssignmentToNewKeyGrowsMap) {
    EXPECT_EQ(Run<int>(
        "m = {\"a\": 1}\n"
        "m[\"b\"] = 5\n"
        "m.size()"),
        2);
}

TEST_F(SigmaRangeTests, SumValuesByIteratingKeys) {
    EXPECT_EQ(Run<int>(
        "m = {\"a\": 1, \"b\": 2, \"c\": 3}\n"
        "t = 0\n"
        "for k in m.keys()\n"
        "    t += m[k]\n"
        "t"),
        6);
}

TEST_F(SigmaRangeTests, NestedMapLookup) {
    EXPECT_EQ(Run<int>(
        "m: Map[str, Map[str, int]] = {\"a\": {\"b\": 3}}\n"
        "inner = m[\"a\"]\n"
        "inner[\"b\"]"),
        3);
}

TEST_F(SigmaRangeTests, GridOfLists) {
    EXPECT_EQ(Run<int>(
        "rows: List[List[int]] = []\n"
        "for i = 0; i < 3; i += 1\n"
        "    row: List[int] = []\n"
        "    for j = 0; j < 3; j += 1\n"
        "        row.push(i * 3 + j)\n"
        "    rows.push(row)\n"
        "rows[2][1]"),
        7);
}

TEST_F(SigmaRangeTests, ListsWithSameElementsAreEqual) {
    EXPECT_EQ(Run<bool>(
        "a = [1, 2]\n"
        "b = [1, 2]\n"
        "r = a == b\n"
        "r"),
        true);
}

TEST_F(SigmaRangeTests, AckermannOfTwoAndThree) {
    EXPECT_EQ(Run<int>(
        "fun ack(m: int, n: int) -> int\n"
        "    if m == 0\n"
        "        return n + 1\n"
        "    if n == 0\n"
        "        return ack(m - 1, 1)\n"
        "    return ack(m - 1, ack(m, n - 1))\n"
        "ack(2, 3)"),
        9);
}

TEST_F(SigmaRangeTests, PowerBySquaring) {
    EXPECT_EQ(Run<int>(
        "fun pow(b: int, e: int) -> int\n"
        "    if e == 0\n"
        "        return 1\n"
        "    half = pow(b, e / 2)\n"
        "    if e % 2 == 0\n"
        "        return half * half\n"
        "    return half * half * b\n"
        "pow(3, 13)"),
        1594323);
}

TEST_F(SigmaRangeTests, FibonacciOfFifteen) {
    EXPECT_EQ(Run<int>(
        "fun fib(n: int) -> int\n"
        "    if n < 2\n"
        "        return n\n"
        "    return fib(n - 1) + fib(n - 2)\n"
        "fib(15)"),
        610);
}

TEST_F(SigmaRangeTests, RecursiveListBuilder) {
    EXPECT_EQ(Run<int>(
        "fun range(n: int) -> List[int]\n"
        "    if n == 0\n"
        "        xs: List[int] = []\n"
        "        return xs\n"
        "    ys = range(n - 1)\n"
        "    ys.push(n)\n"
        "    return ys\n"
        "r = range(5)\n"
        "r[4] * 10 + r.size()"),
        55);
}

TEST_F(SigmaRangeTests, InnerBreakOnlyLeavesTheInnerLoop) {
    EXPECT_EQ(Run<int>(
        "t = 0\n"
        "for i = 0; i < 3; i += 1\n"
        "    for j = 0; j < 10; j += 1\n"
        "        if j == 2\n"
        "            break\n"
        "        t += 1\n"
        "t"),
        6);
}

TEST_F(SigmaRangeTests, InnerContinueSkipsEvenColumns) {
    EXPECT_EQ(Run<int>(
        "t = 0\n"
        "for i = 0; i < 3; i += 1\n"
        "    for j = 0; j < 4; j += 1\n"
        "        if j % 2 == 0\n"
        "            continue\n"
        "        t += 1\n"
        "t"),
        6);
}

TEST_F(SigmaRangeTests, DoWhileContinueStillTestsTheCondition) {
    EXPECT_EQ(Run<int>(
        "i = 0\n"
        "t = 0\n"
        "do\n"
        "    i += 1\n"
        "    if i == 2\n"
        "        continue\n"
        "    t += i\n"
        "while i < 4\n"
        "t"),
        8);
}

TEST_F(SigmaRangeTests, DoWhileBreakLeavesAnInfiniteLoop) {
    EXPECT_EQ(Run<int>(
        "i = 0\n"
        "do\n"
        "    i += 1\n"
        "    if i == 3\n"
        "        break\n"
        "while true\n"
        "i"),
        3);
}

TEST_F(SigmaRangeTests, AnyParameterTakesAnyType) {
    EXPECT_EQ(Run<int>(
        "fun show(x: any) -> int\n"
        "    return 1\n"
        "show(5) + show(\"s\")"),
        2);
}

TEST_F(SigmaRangeTests, AnyListHoldsMixedElements) {
    EXPECT_EQ(Run<int>(
        "xs: List[any] = [1, \"a\"]\n"
        "xs.size()"),
        2);
}

TEST_F(SigmaRangeTests, NegativeLiteralsInAList) {
    EXPECT_EQ(Run<int>(
        "t = 0\n"
        "for x in [-1, -2, -3]\n"
        "    t += x\n"
        "t"),
        -6);
}

TEST_F(SigmaRangeTests, SubtractingANegative) {
    EXPECT_EQ(Run<int>(
        "r = 2 - -3\n"
        "r"),
        5);
}

TEST_F(SigmaRangeTests, NotBindsTighterThanEquality) {
    EXPECT_EQ(Run<bool>(
        "r = !true == false\n"
        "r"),
        true);
}

TEST_F(SigmaRangeTests, IntegerPlusFloatDivision) {
    EXPECT_EQ(Run<float>(
        "r = 7 / 2.0\n"
        "r"),
        3.5f);
}

TEST_F(SigmaRangeTests, EveryCompoundOperator) {
    EXPECT_EQ(Run<int>(
        "x = 10\n"
        "x -= 1\n"
        "x *= 3\n"
        "x /= 2\n"
        "x %= 5\n"
        "x"),
        3);
}

TEST_F(SigmaRangeTests, CallInsideWhileCondition) {
    EXPECT_EQ(Run<int>(
        "fun lt(a: int, b: int) -> bool\n"
        "    return a < b\n"
        "i = 0\n"
        "while lt(i, 5)\n"
        "    i += 1\n"
        "i"),
        5);
}

TEST_F(SigmaRangeTests, SixParameterFunction) {
    EXPECT_EQ(Run<int>(
        "fun f(a: int, b: int, c: int, d: int, e: int, g: int) -> int\n"
        "    return a + b * 2 + c * 3 + d * 4 + e * 5 + g * 6\n"
        "f(1, 1, 1, 1, 1, 1)"),
        21);
}

TEST_F(SigmaRangeTests, IntIsThirtyTwoBitAndWraps) {
    EXPECT_EQ(Run<int>(
        "r = 2000000000 + 2000000000\n"
        "r"),
        -294967296);
}

TEST_F(SigmaRangeTests, UnclosedBracket) {
    ExpectFirstError(
        "x = (1 + 2",
        "1:1: unclosed bracket at end of input");
}

TEST_F(SigmaRangeTests, MissingIndentedBlock) {
    ExpectFirstError(
        "if true\n"
        "x = 1\n",
        "2:1: expected an indented block (found 'x')");
}

TEST_F(SigmaRangeTests, StrayIndent) {
    ExpectFirstError(
        "  x = 1\n",
        "1:3: indentation must be a tab or a multiple of four spaces");
}

TEST_F(SigmaRangeTests, UnexpectedIndent) {
    ExpectFirstError(
        "if true\n"
        "        x = 1\n"
        "    y = 2\n",
        "2:9: unexpected indent");
}

TEST_F(SigmaRangeTests, AnonymousFunctionIsRejected) {
    ExpectFirstError(
        "fun (a: int)\n"
        "    return a\n",
        "1:1: anonymous functions are not supported in Sigma yet (found 'fun')");
}

TEST_F(SigmaRangeTests, ElseWithoutIf) {
    ExpectFirstError(
        "else\n"
        "    x = 1\n",
        "1:1: 'else' without a matching 'if' (found 'else')");
}

TEST_F(SigmaRangeTests, ForLoopMissingSecondSemicolon) {
    ExpectFirstError(
        "for i = 0; i < 3\n"
        "    print(i)\n",
        "1:17: expected ';' after the for-loop condition (found end of line)");
}

TEST_F(SigmaRangeTests, DuplicateFunction) {
    ExpectFirstError(
        "fun f()\n"
        "    print(1)\n"
        "fun f()\n"
        "    print(2)\n",
        "3:5: 'f' is already declared");
}

TEST_F(SigmaRangeTests, FunctionValueArityIsChecked) {
    ExpectFirstError(
        "fun inc(n: int) -> int\n"
        "    return n + 1\n"
        "f: fun(int) -> int = inc\n"
        "f(1, 2)\n",
        "4:2: 'f' expects 1 argument, got 2");
}

TEST_F(SigmaRangeTests, BoolsAreNotOrdered) {
    ExpectFirstError(
        "b = true < false\n",
        "1:10: operator '<' cannot be applied to bool and bool");
}

TEST_F(SigmaRangeTests, ListIndexMustBeInt) {
    ExpectFirstError(
        "xs = [1]\n"
        "v = xs[1.0]\n",
        "2:8: list index: expected int, got float");
}

TEST_F(SigmaRangeTests, StringsCannotBeIndexed) {
    ExpectFirstError(
        "s = \"abc\"\n"
        "c = s[1]\n",
        "2:6: str cannot be indexed");
}

TEST_F(SigmaRangeTests, CompoundAssignNeedsAName) {
    ExpectFirstError(
        "xs = [1]\n"
        "xs[0] += 1\n",
        "2:7: '+=' needs a plain name on the left");
}

TEST_F(SigmaRangeTests, MapValuesMustAgree) {
    ExpectFirstError(
        "m = {\"a\": 1, \"b\": \"x\"}",
        "1:5: map values have different types: int and str");
}


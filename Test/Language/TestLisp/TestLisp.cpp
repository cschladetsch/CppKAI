#include <KAI/Console/Console.h>
#include <KAI/Language/Common/Language.h>
#include <gtest/gtest.h>

#include "TestLangCommon.h"

using namespace kai;

// End-to-end tests for the Lisp front-end: source text through the lexer,
// parser and translator, executed on the same Executor that runs Pi and Rho.
// AssertResult clears the data stack, runs the program, and checks the value
// left on top -- so each script has to end in an expression.
struct TestLisp : TestLangCommon {
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(Language::Lisp);
    }
};

// ---------------------------------------------------------------- literals

TEST_F(TestLisp, IntLiteral) { AssertResult("42", 42); }
TEST_F(TestLisp, ZeroLiteral) { AssertResult("0", 0); }
TEST_F(TestLisp, NegativeIntLiteral) { AssertResult("-7", -7); }
TEST_F(TestLisp, FloatLiteral) { AssertResult("2.5", 2.5f); }
TEST_F(TestLisp, NegativeFloatLiteral) { AssertResult("-1.25", -1.25f); }
TEST_F(TestLisp, BoolTrueLiteral) { AssertResult("#t", true); }
TEST_F(TestLisp, BoolFalseLiteral) { AssertResult("#f", false); }
TEST_F(TestLisp, StringLiteral) { AssertResult("\"hello\"", String("hello")); }

// ------------------------------------------------ addition and subtraction

TEST_F(TestLisp, AddTwo) { AssertResult("(+ 1 2)", 3); }
TEST_F(TestLisp, AddThree) { AssertResult("(+ 1 2 3)", 6); }
TEST_F(TestLisp, AddMany) { AssertResult("(+ 1 2 3 4 5 6 7 8 9 10)", 55); }
TEST_F(TestLisp, AddSingleArgIsIdentity) { AssertResult("(+ 5)", 5); }
TEST_F(TestLisp, AddNoArgsIsZero) { AssertResult("(+)", 0); }
TEST_F(TestLisp, AddNegativeOperand) { AssertResult("(+ 5 -3)", 2); }
TEST_F(TestLisp, SubTwo) { AssertResult("(- 10 3)", 7); }
TEST_F(TestLisp, SubFoldsLeftAssociatively) { AssertResult("(- 20 5 3)", 12); }
TEST_F(TestLisp, SubSingleArgNegates) { AssertResult("(- 5)", -5); }

// ------------------------------------ multiplication, division and modulo

TEST_F(TestLisp, MulTwo) { AssertResult("(* 3 4)", 12); }
TEST_F(TestLisp, MulThree) { AssertResult("(* 2 3 4)", 24); }
TEST_F(TestLisp, MulSingleArgIsIdentity) { AssertResult("(* 7)", 7); }
TEST_F(TestLisp, MulNoArgsIsOne) { AssertResult("(*)", 1); }
TEST_F(TestLisp, MulByZero) { AssertResult("(* 5 0)", 0); }
TEST_F(TestLisp, DivExact) { AssertResult("(/ 12 3)", 4); }
TEST_F(TestLisp, DivFoldsLeftAssociatively) { AssertResult("(/ 100 5 2)", 10); }
TEST_F(TestLisp, ModBasic) { AssertResult("(mod 10 3)", 1); }
TEST_F(TestLisp, ModZeroRemainder) { AssertResult("(mod 9 3)", 0); }

// ------------------------------------------------ nesting and mixed arity

TEST_F(TestLisp, NestedMulOverAdd) { AssertResult("(* (+ 1 2) 4)", 12); }
TEST_F(TestLisp, NestedAddOverMul) { AssertResult("(+ (* 2 3) 4)", 10); }
TEST_F(TestLisp, DeeplyNested) {
    AssertResult("(+ 1 (+ 2 (+ 3 (+ 4 5))))", 15);
}
TEST_F(TestLisp, MixedOperators) { AssertResult("(- (* 6 7) (/ 10 5))", 40); }
TEST_F(TestLisp, NestedFloatArithmetic) {
    AssertResult("(+ 1.5 (* 0.5 1.5))", 2.25f);
}
TEST_F(TestLisp, NestedModulo) { AssertResult("(mod (+ 10 5) 4)", 3); }

// -------------------------------------------------------------- comparison

TEST_F(TestLisp, LessTrue) { AssertResult("(< 1 2)", true); }
TEST_F(TestLisp, LessFalse) { AssertResult("(< 2 1)", false); }
TEST_F(TestLisp, GreaterTrue) { AssertResult("(> 5 3)", true); }
TEST_F(TestLisp, GreaterFalse) { AssertResult("(> 3 5)", false); }
TEST_F(TestLisp, LessEqualAtBoundary) { AssertResult("(<= 2 2)", true); }
TEST_F(TestLisp, GreaterEqualAtBoundary) { AssertResult("(>= 3 3)", true); }
TEST_F(TestLisp, EqualTrue) { AssertResult("(= 4 4)", true); }
TEST_F(TestLisp, EqualFalse) { AssertResult("(= 4 5)", false); }
TEST_F(TestLisp, NotEqualTrue) { AssertResult("(/= 4 5)", true); }

// ------------------------------------------------------------------- logic

TEST_F(TestLisp, NotOfFalse) { AssertResult("(not #f)", true); }
TEST_F(TestLisp, NotOfTrue) { AssertResult("(not #t)", false); }
TEST_F(TestLisp, AndBothTrue) { AssertResult("(and #t #t)", true); }
TEST_F(TestLisp, AndWithFalse) { AssertResult("(and #t #f)", false); }
TEST_F(TestLisp, OrWithTrue) { AssertResult("(or #f #t)", true); }
TEST_F(TestLisp, OrBothFalse) { AssertResult("(or #f #f)", false); }

// ---------------------------------------------------------------------- if

TEST_F(TestLisp, IfTakesThenBranch) { AssertResult("(if #t 1 2)", 1); }
TEST_F(TestLisp, IfTakesElseBranch) { AssertResult("(if #f 1 2)", 2); }
TEST_F(TestLisp, IfWithoutElse) { AssertResult("(if #t 5)", 5); }
TEST_F(TestLisp, IfOnComparison) { AssertResult("(if (> 1 2) 10 20)", 20); }
TEST_F(TestLisp, IfNestedInArithmetic) {
    AssertResult("(+ 1 (if #t 10 20))", 11);
}
TEST_F(TestLisp, IfNestedInIf) { AssertResult("(if #t (if #f 1 2) 3)", 2); }

// -------------------------------------------------------------------- cond

TEST_F(TestLisp, CondTakesFirstClause) {
    AssertResult("(cond ((= 1 1) 10) (else 20))", 10);
}
TEST_F(TestLisp, CondTakesLaterClause) {
    AssertResult("(cond ((> 1 2) 10) ((< 1 2) 20) (else 30))", 20);
}
TEST_F(TestLisp, CondFallsBackToElse) {
    AssertResult("(cond ((= 1 2) 5) (else 9))", 9);
}
TEST_F(TestLisp, CondWithThreeClauses) {
    AssertResult("(cond ((= 1 2) 1) ((= 2 3) 2) ((= 3 3) 3) (else 4))", 3);
}
TEST_F(TestLisp, CondClauseComputesValue) {
    AssertResult("(cond ((< 5 1) 0) (else (* 6 7)))", 42);
}

// --------------------------------------------------------------- variables

TEST_F(TestLisp, DefineThenRead) { AssertResult("(define x 7) x", 7); }
TEST_F(TestLisp, DefineThenUseInExpression) {
    AssertResult("(define x 7) (* x 3)", 21);
}
TEST_F(TestLisp, DefineTwoVariables) {
    AssertResult("(define a 3) (define b 4) (+ a b)", 7);
}
TEST_F(TestLisp, DefineFromExpression) {
    AssertResult("(define x (+ 2 3)) x", 5);
}
TEST_F(TestLisp, SetBangUpdatesBinding) {
    AssertResult("(define x 1) (set! x 9) x", 9);
}
TEST_F(TestLisp, SetBangWithArithmetic) {
    AssertResult("(define x 4) (set! x (* x x)) x", 16);
}

// ---------------------------------------------------- functions and lambda

TEST_F(TestLisp, LambdaCalledImmediately) {
    AssertResult("((lambda (n) (* n n)) 6)", 36);
}
TEST_F(TestLisp, LambdaStoredThenCalled) {
    AssertResult("(define sq (lambda (n) (* n n))) (sq 6)", 36);
}
TEST_F(TestLisp, LambdaWithTwoArgs) {
    AssertResult("(define f (lambda (a b) (- a b))) (f 10 4)", 6);
}
TEST_F(TestLisp, DefineFunctionOneArg) {
    AssertResult("(define (dbl n) (* n 2)) (dbl 21)", 42);
}
TEST_F(TestLisp, DefineFunctionTwoArgs) {
    AssertResult("(define (add2 a b) (+ a b)) (add2 3 4)", 7);
}
TEST_F(TestLisp, DefineFunctionThreeArgs) {
    AssertResult("(define (add3 a b c) (+ a b c)) (add3 1 2 3)", 6);
}
TEST_F(TestLisp, FunctionCallingFunction) {
    AssertResult(
        "(define (dbl n) (* n 2)) (define (quad n) (dbl (dbl n))) (quad 5)",
        20);
}

// --------------------------------------------------------------- recursion

TEST_F(TestLisp, RecursiveFactorial) {
    AssertResult(
        "(define (fact n) (if (<= n 1) 1 (* n (fact (- n 1))))) (fact 5)", 120);
}
TEST_F(TestLisp, RecursiveFibonacci) {
    AssertResult(
        "(define (fib n) (if (< n 2) n (+ (fib (- n 1)) (fib (- n 2)))))"
        "(fib 10)",
        55);
}
TEST_F(TestLisp, RecursiveSumTo) {
    AssertResult(
        "(define (sumto n) (if (= n 0) 0 (+ n (sumto (- n 1))))) (sumto 10)",
        55);
}
TEST_F(TestLisp, RecursiveGcd) {
    AssertResult(
        "(define (gcd a b) (if (= b 0) a (gcd b (mod a b)))) (gcd 48 18)", 6);
}
TEST_F(TestLisp, RecursivePower) {
    AssertResult(
        "(define (pw b e) (if (= e 0) 1 (* b (pw b (- e 1))))) (pw 2 8)", 256);
}
TEST_F(TestLisp, MutualRecursion) {
    AssertResult(
        "(define (isodd n) (if (= n 0) #f (iseven (- n 1))))"
        "(define (iseven n) (if (= n 0) #t (isodd (- n 1))))"
        "(isodd 7)",
        true);
}

// ---------------------------------------------------------------- closures
//
// A lambda outliving the frame that defined its free variables. These need the
// lexical capture in Executor::CaptureClosure; the dynamic context-stack walk
// alone cannot resolve them once the creating frame has been popped.

TEST_F(TestLisp, ClosureCapturesEnclosingArg) {
    AssertResult(
        "(define (mk a) (lambda (b) (+ a b))) (define f (mk 10)) (f 5)", 15);
}
TEST_F(TestLisp, ClosuresFromSameLambdaAreDistinct) {
    AssertResult(
        "(define (mk a) (lambda (b) (+ a b)))"
        "(define f (mk 10)) (define g (mk 100))"
        "(+ (f 1) (g 1))",
        112);
}
TEST_F(TestLisp, LaterClosureDoesNotClobberEarlier) {
    AssertResult(
        "(define (mk a) (lambda (b) (+ a b)))"
        "(define f (mk 10)) (define g (mk 100)) (f 5)",
        15);
}
TEST_F(TestLisp, NestedClosureReachesTwoFramesOut) {
    AssertResult(
        "(define (outer a) (lambda (b) (lambda (c) (+ (+ a b) c))))"
        "(define f ((outer 1) 2)) (f 3)",
        6);
}
TEST_F(TestLisp, ClosureCapturesComputedValue) {
    AssertResult(
        "(define (mk a) (lambda (b) (* a b)))"
        "(define trip (mk (+ 1 2))) (trip 5)",
        15);
}
TEST_F(TestLisp, ClosurePassedAsArgument) {
    AssertResult(
        "(define (mk a) (lambda (b) (+ a b)))"
        "(define (call f v) (f v))"
        "(call (mk 7) 3)",
        10);
}

// --------------------------------------------------------------------- let

TEST_F(TestLisp, LetSingleBinding) {
    AssertResult("(let ((a 5)) (* a 2))", 10);
}
TEST_F(TestLisp, LetTwoBindings) {
    AssertResult("(let ((a 2) (b 3)) (* a b))", 6);
}
TEST_F(TestLisp, LetNested) {
    AssertResult("(let ((a 1)) (let ((b 2)) (+ a b)))", 3);
}
TEST_F(TestLisp, LetBindingsAreSequential) {
    // let* semantics: a later binding can see an earlier one.
    AssertResult("(let ((a 2) (b (* a 3))) b)", 6);
}

// ------------------------------------------------------------ begin, while

TEST_F(TestLisp, BeginSingleForm) { AssertResult("(begin (+ 1 1))", 2); }
TEST_F(TestLisp, BeginRunsFormsInOrder) {
    AssertResult("(begin (define x 3) (* x x))", 9);
}
TEST_F(TestLisp, WhileAccumulates) {
    AssertResult(
        "(define i 0) (define s 0)"
        "(while (< i 5) (set! s (+ s i)) (set! i (+ i 1)))"
        "s",
        10);
}
TEST_F(TestLisp, WhileCountsDown) {
    AssertResult(
        "(define n 5) (define c 0)"
        "(while (> n 0) (set! n (- n 1)) (set! c (+ c 1)))"
        "c",
        5);
}

// --------------------------------------------------------- lists and quote

TEST_F(TestLisp, ListSize) { AssertResult("(size (list 1 2 3))", 3); }
TEST_F(TestLisp, EmptyListSize) { AssertResult("(size (list))", 0); }
TEST_F(TestLisp, QuotedListSize) { AssertResult("(size (quote (1 2)))", 2); }
TEST_F(TestLisp, QuoteShorthandSize) { AssertResult("(size '(1 2 3))", 3); }
TEST_F(TestLisp, NilIsEmptyList) { AssertResult("(size nil)", 0); }
TEST_F(TestLisp, StringSize) { AssertResult("(size \"abcd\")", 4); }

// -------------------------------------------------------- comments, layout

TEST_F(TestLisp, LeadingComment) { AssertResult("; a comment\n(+ 2 3)", 5); }
TEST_F(TestLisp, TrailingComment) { AssertResult("(+ 2 3) ; done", 5); }
TEST_F(TestLisp, BlankLinesAndIndentationIgnored) {
    // Also covers a program spanning several lines.
    AssertResult("(define x 1)\n\n   (+ x\n      4)", 5);
}

// ==========================================================================
// Complex programs
// ==========================================================================
//
// Multi-line Lisp exercising the pieces against each other: higher-order
// functions, closures that outlive their creator, deep and mutual recursion,
// and iterative algorithms built from while/set!. Each still asserts a
// computed value, so a wrong answer fails rather than merely not crashing.

// ------------------------------------------- higher-order and composition

TEST_F(TestLisp, ComposeTwoFunctions) {
    AssertResult(
        "(define (compose f g) (lambda (x) (f (g x))))"
        "(define (inc n) (+ n 1)) (define (dbl n) (* n 2))"
        "((compose inc dbl) 5)",
        11);
}
TEST_F(TestLisp, ComposeThreeFunctions) {
    AssertResult(
        "(define (comp f g) (lambda (x) (f (g x))))"
        "(define (i n) (+ n 1)) (define (d n) (* n 2)) (define (s n) (* n n))"
        "((comp i (comp d s)) 3)",
        19);
}
TEST_F(TestLisp, CurriedThreeDeep) {
    AssertResult(
        "(define (a x) (lambda (y) (lambda (z) (+ (+ x y) z))))"
        "(((a 1) 2) 3)",
        6);
}
TEST_F(TestLisp, NestedClosureFourLevels) {
    AssertResult(
        "(define (a x) (lambda (y) (lambda (z) (lambda (w) (+ (+ x y) (+ z w))))))"
        "((((a 1) 2) 3) 4)",
        10);
}
TEST_F(TestLisp, HigherOrderTwiceApplied) {
    AssertResult(
        "(define (twice f) (lambda (x) (f (f x))))"
        "(define (q f) (twice (twice f)))"
        "((q (lambda (n) (+ n 1))) 0)",
        4);
}
TEST_F(TestLisp, ApplyFunctionNTimes) {
    AssertResult(
        "(define (applyn f n x) (if (= n 0) x (applyn f (- n 1) (f x))))"
        "(applyn (lambda (v) (* v 2)) 5 1)",
        32);
}
TEST_F(TestLisp, FoldOverRangeRecursively) {
    AssertResult(
        "(define (fold f acc n) (if (= n 0) acc (fold f (f acc n) (- n 1))))"
        "(fold (lambda (a v) (+ a v)) 0 10)",
        55);
}
TEST_F(TestLisp, DispatchTableOfLambdas) {
    AssertResult(
        "(define (op name) (cond ((= name 1) (lambda (a b) (+ a b)))"
        "((= name 2) (lambda (a b) (* a b))) (else (lambda (a b) 0))))"
        "(+ ((op 1) 3 4) ((op 2) 3 4))",
        19);
}
TEST_F(TestLisp, HigherOrderReturnedFromCond) {
    AssertResult(
        "(define (pick k) (cond ((= k 1) (lambda (x) (+ x 1)))"
        "(else (lambda (x) (* x 10)))))"
        "((pick 2) 7)",
        70);
}
TEST_F(TestLisp, YCombinatorFactorial) {
    // Recursion built purely from closures, with no self-reference by name.
    AssertResult(
        "(define Y (lambda (f) ((lambda (x) (f (lambda (v) ((x x) v))))"
        "(lambda (x) (f (lambda (v) ((x x) v)))))))"
        "(define fac (Y (lambda (self)"
        "  (lambda (n) (if (= n 0) 1 (* n (self (- n 1))))))))"
        "(fac 5)",
        120);
}

// -------------------------------------------------------- escaping closures

TEST_F(TestLisp, ClosureOverLetBinding) {
    AssertResult("(define f (let ((k 9)) (lambda (x) (+ k x)))) (f 1)", 10);
}
TEST_F(TestLisp, ClosureOverLetInsideFunction) {
    AssertResult(
        "(define (f a) (let ((b (* a 2))) (lambda (c) (+ b c))))"
        "(define g (f 5)) (g 3)",
        13);
}
TEST_F(TestLisp, ClosureOverLetReturnedDirectly) {
    AssertResult(
        "(define (outer) (let ((base 100)) (lambda (d) (- base d))))"
        "((outer) 40)",
        60);
}
TEST_F(TestLisp, TwoClosuresFromOneFactoryStayIndependent) {
    AssertResult(
        "(define (adder n) (lambda (x) (+ x n)))"
        "(define a5 (adder 5)) (define a9 (adder 9))"
        "(* (a5 1) (a9 1))",
        60);
}
TEST_F(TestLisp, ZeroArgClosureReadsCapturedBinding) {
    AssertResult(
        "(define (mk) (define n 5) (lambda () n)) (define c (mk)) (c)", 5);
}
TEST_F(TestLisp, SetBangOnCapturedArgWithinCall) {
    AssertResult(
        "(define (mk a) (lambda (b) (set! a (+ a b)) a))"
        "(define f (mk 10)) (f 5)",
        15);
}

// ------------------------------------------------------- deeper recursion

TEST_F(TestLisp, AckermannTwoThree) {
    AssertResult(
        "(define (ack m n) (cond ((= m 0) (+ n 1))"
        "((= n 0) (ack (- m 1) 1))"
        "(else (ack (- m 1) (ack m (- n 1))))))"
        "(ack 2 3)",
        9);
}
TEST_F(TestLisp, AckermannTwoTwo) {
    AssertResult(
        "(define (ack m n) (cond ((= m 0) (+ n 1))"
        "((= n 0) (ack (- m 1) 1))"
        "(else (ack (- m 1) (ack m (- n 1))))))"
        "(ack 2 2)",
        7);
}
TEST_F(TestLisp, TailRecursiveFactorial) {
    AssertResult(
        "(define (fa n acc) (if (= n 0) acc (fa (- n 1) (* acc n))))"
        "(fa 6 1)",
        720);
}
TEST_F(TestLisp, TailRecursiveFibonacci) {
    AssertResult(
        "(define (fi n a b) (if (= n 0) a (fi (- n 1) b (+ a b))))"
        "(fi 20 0 1)",
        6765);
}
TEST_F(TestLisp, CollatzStepCount) {
    AssertResult(
        "(define (col n s) (cond ((= n 1) s)"
        "((= (mod n 2) 0) (col (/ n 2) (+ s 1)))"
        "(else (col (+ (* 3 n) 1) (+ s 1)))))"
        "(col 27 0)",
        111);
}
TEST_F(TestLisp, RecursionDepthOneHundred) {
    AssertResult(
        "(define (cnt n) (if (= n 0) 0 (+ 1 (cnt (- n 1))))) (cnt 100)", 100);
}
TEST_F(TestLisp, ThreeWayMutualRecursionEndsInSecond) {
    // fa 7 -> fb 6 -> fc 5 -> fa 4 -> fb 3 -> fc 2 -> fa 1 -> fb 0 -> 1
    AssertResult(
        "(define (fa n) (if (= n 0) 0 (fb (- n 1))))"
        "(define (fb n) (if (= n 0) 1 (fc (- n 1))))"
        "(define (fc n) (if (= n 0) 2 (fa (- n 1))))"
        "(fa 7)",
        1);
}
TEST_F(TestLisp, ThreeWayMutualRecursionEndsInFirst) {
    // fa 3 -> fb 2 -> fc 1 -> fa 0 -> 0
    AssertResult(
        "(define (fa n) (if (= n 0) 0 (fb (- n 1))))"
        "(define (fb n) (if (= n 0) 1 (fc (- n 1))))"
        "(define (fc n) (if (= n 0) 2 (fa (- n 1))))"
        "(fa 3)",
        0);
}

// ------------------------------------------------------ numeric algorithms

TEST_F(TestLisp, ModularExponentiation) {
    AssertResult(
        "(define (pm b e m) (if (= e 0) 1 (mod (* b (pm b (- e 1) m)) m)))"
        "(pm 3 7 13)",
        3);
}
TEST_F(TestLisp, ExponentiationBySquaring) {
    AssertResult(
        "(define (p b e) (cond ((= e 0) 1)"
        "((= (mod e 2) 0) (let ((h (p b (/ e 2)))) (* h h)))"
        "(else (* b (p b (- e 1))))))"
        "(p 3 5)",
        243);
}
TEST_F(TestLisp, LeastCommonMultipleViaGcd) {
    AssertResult(
        "(define (g a b) (if (= b 0) a (g b (mod a b))))"
        "(define (l a b) (/ (* a b) (g a b))) (l 12 18)",
        36);
}
TEST_F(TestLisp, BinomialCoefficient) {
    AssertResult(
        "(define (c n k) (cond ((= k 0) 1) ((= n k) 1)"
        "(else (+ (c (- n 1) (- k 1)) (c (- n 1) k)))))"
        "(c 6 2)",
        15);
}
TEST_F(TestLisp, CatalanNumber) {
    AssertResult(
        "(define (f n) (if (<= n 1) 1 (* n (f (- n 1)))))"
        "(define (cat n) (/ (f (* 2 n)) (* (f (+ n 1)) (f n)))) (cat 4)",
        14);
}
TEST_F(TestLisp, PrimalityByTrialDivision) {
    AssertResult(
        "(define (chk n d) (cond ((> (* d d) n) #t) ((= (mod n d) 0) #f)"
        "(else (chk n (+ d 1)))))"
        "(define (prime n) (if (< n 2) #f (chk n 2))) (prime 97)",
        true);
}
TEST_F(TestLisp, PerfectNumberCheck) {
    AssertResult(
        "(define (sd n d a) (cond ((>= d n) a)"
        "((= (mod n d) 0) (sd n (+ d 1) (+ a d)))"
        "(else (sd n (+ d 1) a))))"
        "(= (sd 28 1 0) 28)",
        true);
}
TEST_F(TestLisp, ReverseDigitsOfInteger) {
    AssertResult(
        "(define (rv n a) (if (= n 0) a (rv (/ n 10) (+ (* a 10) (mod n 10)))))"
        "(rv 1234 0)",
        4321);
}
TEST_F(TestLisp, SumOfDigits) {
    AssertResult(
        "(define (ds n) (if (= n 0) 0 (+ (mod n 10) (ds (/ n 10)))))"
        "(ds 9876)",
        30);
}
TEST_F(TestLisp, CountDigits) {
    AssertResult(
        "(define (cd n) (if (< n 10) 1 (+ 1 (cd (/ n 10))))) (cd 90210)", 5);
}
TEST_F(TestLisp, MultiplyByRepeatedAddition) {
    AssertResult(
        "(define (m a b) (if (= b 0) 0 (+ a (m a (- b 1))))) (m 7 6)", 42);
}
TEST_F(TestLisp, BinarySearchOnPredicate) {
    AssertResult(
        "(define (bs lo hi) (if (>= lo hi) lo"
        "(let ((mid (/ (+ lo hi) 2)))"
        "(if (< (* mid mid) 200) (bs (+ mid 1) hi) (bs lo mid)))))"
        "(bs 0 100)",
        15);
}
TEST_F(TestLisp, RecursiveMinOverRange) {
    AssertResult(
        "(define (mn n acc) (if (= n 0) acc (mn (- n 1) (min acc (* n (- n 7))))))"
        "(mn 10 0)",
        -12);
}
TEST_F(TestLisp, MaxOfThreeNested) {
    AssertResult(
        "(define (m3 a b c) (if (> a b) (if (> a c) a c) (if (> b c) b c)))"
        "(m3 4 9 2)",
        9);
}

// ------------------------------------------------- iterative with while

TEST_F(TestLisp, IterativeFibonacciWithWhile) {
    AssertResult(
        "(define a 0) (define b 1) (define i 0)"
        "(while (< i 10) (define t (+ a b)) (set! a b) (set! b t)"
        "  (set! i (+ i 1)))"
        "a",
        55);
}
TEST_F(TestLisp, IterativeFactorialWithWhile) {
    AssertResult(
        "(define n 6) (define f 1)"
        "(while (> n 1) (set! f (* f n)) (set! n (- n 1))) f",
        720);
}
TEST_F(TestLisp, TriangularNumberWithWhile) {
    AssertResult(
        "(define i 1) (define t 0)"
        "(while (<= i 10) (set! t (+ t i)) (set! i (+ i 1))) t",
        55);
}
TEST_F(TestLisp, IntegerSquareRootWithWhile) {
    AssertResult(
        "(define r 0) (while (<= (* (+ r 1) (+ r 1)) 144) (set! r (+ r 1))) r",
        12);
}
TEST_F(TestLisp, SumOfEvensWithWhile) {
    AssertResult(
        "(define i 0) (define s 0)"
        "(while (< i 20) (if (= (mod i 2) 0) (set! s (+ s i)))"
        "  (set! i (+ i 1)))"
        "s",
        90);
}
TEST_F(TestLisp, NestedWhileCountsPrimes) {
    AssertResult(
        "(define n 2) (define count 0)"
        "(while (< n 20)"
        "  (define d 2) (define isp 1)"
        "  (while (< d n) (if (= (mod n d) 0) (set! isp 0)) (set! d (+ d 1)))"
        "  (if (= isp 1) (set! count (+ count 1)))"
        "  (set! n (+ n 1)))"
        "count",
        8);
}
TEST_F(TestLisp, NestedWhileMultiplicationTable) {
    AssertResult(
        "(define r 1) (define t 0)"
        "(while (<= r 3)"
        "  (define c 1)"
        "  (while (<= c 3) (set! t (+ t (* r c))) (set! c (+ c 1)))"
        "  (set! r (+ r 1)))"
        "t",
        36);
}
TEST_F(TestLisp, WhileWithNestedIfElseBranches) {
    AssertResult(
        "(define i 0) (define ev 0) (define od 0)"
        "(while (< i 10)"
        "  (if (= (mod i 2) 0) (set! ev (+ ev 1)) (set! od (+ od 1)))"
        "  (set! i (+ i 1)))"
        "(* ev od)",
        25);
}

// ------------------------------------------------- floats, scoping, nesting

TEST_F(TestLisp, NewtonsMethodSquareRoot) {
    AssertResult(
        "(define (it g n c) (if (= c 0) g (it (/ (+ g (/ n g)) 2.0) n (- c 1))))"
        "(it 1.0 16.0 6)",
        4.0f);
}
TEST_F(TestLisp, ExactBinaryFractionSum) {
    AssertResult("(+ 1.0 (+ 0.5 (+ 0.25 0.125)))", 1.875f);
}
TEST_F(TestLisp, NestedLetShadowsOuterBinding) {
    AssertResult("(let ((x 1)) (let ((x 5)) (* x 2)))", 10);
}
TEST_F(TestLisp, DeeplyNestedMixedArithmetic) {
    AssertResult("(* (+ (* (+ 1 2) (- 8 3)) (/ 20 4)) (- (+ 3 4) (* 1 2)))",
                 100);
}

// ==========================================================================
// Containers
// ==========================================================================
//
// An array-oriented Lisp rather than a cons-cell one: the runtime stores
// Arrays, Lists and Maps with integer indexing, not pairs. car and cdr are
// provided because they fall out of Index and ArraySlice, but there is no
// cons -- nth, push! and set-nth! cover the same ground and suit a grid
// better.

TEST_F(TestLisp, ArrayLiteralHasSize) { AssertResult("(size (array 1 2 3))", 3); }
TEST_F(TestLisp, EmptyArrayHasNoSize) { AssertResult("(size (array))", 0); }
TEST_F(TestLisp, NthReadsElement) {
    AssertResult("(nth (array 10 20 30) 1)", 20);
}
TEST_F(TestLisp, NthAtBothEnds) {
    AssertResult("(+ (nth (array 5 6 7) 0) (nth (array 5 6 7) 2))", 12);
}
TEST_F(TestLisp, SetNthOverwritesElement) {
    AssertResult("(define a (array 1 2 3)) (set-nth! a 1 99) (nth a 1)", 99);
}
TEST_F(TestLisp, PushGrowsArray) {
    AssertResult("(define a (array)) (push! a 7) (push! a 8) (size a)", 2);
}
TEST_F(TestLisp, PushThenIndex) {
    AssertResult("(define a (array)) (push! a 7) (push! a 8) (nth a 1)", 8);
}
TEST_F(TestLisp, BuildArrayInWhileLoop) {
    AssertResult(
        "(define a (array)) (define i 0)"
        "(while (< i 10) (push! a (* i i)) (set! i (+ i 1)))"
        "(nth a 7)",
        49);
}
TEST_F(TestLisp, CarReturnsFirst) { AssertResult("(car (array 4 5 6))", 4); }
TEST_F(TestLisp, CdrDropsFirst) {
    AssertResult("(size (cdr (array 4 5 6)))", 2);
}
TEST_F(TestLisp, CarOfCdrIsSecond) {
    AssertResult("(car (cdr (array 4 5 6)))", 5);
}
TEST_F(TestLisp, SliceLength) {
    AssertResult("(size (slice (array 1 2 3 4 5) 1 4))", 3);
}
TEST_F(TestLisp, SliceContents) {
    AssertResult("(nth (slice (array 1 2 3 4 5) 1 4) 0)", 2);
}
TEST_F(TestLisp, StringIsIndexable) {
    AssertResult("(nth \"abc\" 1)", String("b"));
}
TEST_F(TestLisp, DictLookupByKey) {
    AssertResult("(nth (dict \"a\" 1 \"b\" 2) \"b\")", 2);
}
TEST_F(TestLisp, DictKeysCount) {
    AssertResult("(size (keys (dict \"a\" 1 \"b\" 2)))", 2);
}
TEST_F(TestLisp, SumArrayByRecursion) {
    AssertResult(
        "(define a (array 1 2 3 4 5))"
        "(define (sum i acc) (if (>= i (size a)) acc"
        "  (sum (+ i 1) (+ acc (nth a i)))))"
        "(sum 0 0)",
        15);
}
TEST_F(TestLisp, ArrayOfArraysIndexedTwice) {
    AssertResult("(define m (array (array 1 2) (array 3 4)))"
                 "(nth (nth m 1) 0)",
                 3);
}
TEST_F(TestLisp, FlatGridWriteThenRead) {
    // 5x5 grid held flat; write at (x=3,y=2) and read back at y*5+x = 13.
    AssertResult(
        "(define g (array)) (define i 0)"
        "(while (< i 25) (push! g 0) (set! i (+ i 1)))"
        "(set-nth! g (+ (* 2 5) 3) 42)"
        "(nth g 13)",
        42);
}
TEST_F(TestLisp, FlatGridDiagonalSum) {
    AssertResult(
        "(define g (array)) (define i 0)"
        "(while (< i 9) (push! g i) (set! i (+ i 1)))"
        "(define (cell x y) (nth g (+ (* y 3) x)))"
        "(+ (cell 0 0) (+ (cell 1 1) (cell 2 2)))",
        12);
}

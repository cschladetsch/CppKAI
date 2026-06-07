#include <iostream>
#include <vector>
#include <string>
#include <format>
#include <chrono>
#include <ranges>
#include <algorithm>
#include <KAI/Ext/Rang/rang.hpp>

using namespace rang;

// ── helpers ────────────────────────────────────────────────────────────────

void banner() {
    std::cout << style::bold << fg::blue
              << "╔══════════════════════════════╗\n"
              << "║   "
              << fg::cyan  << "xc"
              << fg::blue  << " — C++20 demo app         ║\n"
              << "╚══════════════════════════════╝\n"
              << style::reset;
}

void info (std::string_view msg) {
    std::cout << style::bold << fg::cyan    << "[info]  " << style::reset << msg << '\n';
}
void ok   (std::string_view msg) {
    std::cout << style::bold << fg::green   << "[ ok ]  " << style::reset << msg << '\n';
}
void warn (std::string_view msg) {
    std::cout << style::bold << fg::yellow  << "[warn]  " << style::reset << msg << '\n';
}
void err  (std::string_view msg) {
    std::cout << style::bold << fg::red     << "[err]   " << style::reset << msg << '\n';
}

// ── C++20 demo: ranges pipeline ───────────────────────────────────────────

void demo_ranges() {
    std::cout << '\n' << style::bold << fg::magenta
              << "── ranges demo ──────────────────\n" << style::reset;

    std::vector<int> nums{1,2,3,4,5,6,7,8,9,10};

    auto pipeline = nums
        | std::views::filter([](int n){ return n % 2 == 0; })
        | std::views::transform([](int n){ return n * n; });

    std::cout << fg::yellow << "even squares: " << style::reset;
    for (int v : pipeline)
        std::cout << fg::cyan << v << style::reset << ' ';
    std::cout << '\n';

    ok("ranges pipeline complete");
}

// ── C++20 demo: format + chrono ───────────────────────────────────────────

void demo_format() {
    std::cout << '\n' << style::bold << fg::magenta
              << "── format demo ──────────────────\n" << style::reset;

    auto now = std::chrono::system_clock::now();
    auto t   = std::chrono::system_clock::to_time_t(now);

    info(std::format("built with C++{}", 20));
    info(std::format("unix epoch seconds: {}", t));
    ok("format demo complete");
}

// ── C++20 demo: concepts ──────────────────────────────────────────────────

template<std::integral T>
T square(T x) { return x * x; }

template<std::floating_point T>
T square(T x) { return x * x; }

void demo_concepts() {
    std::cout << '\n' << style::bold << fg::magenta
              << "── concepts demo ────────────────\n" << style::reset;

    std::cout << fg::yellow << "square(7)    = " << style::reset
              << fg::cyan   << square(7)          << style::reset << '\n';
    std::cout << fg::yellow << "square(3.14) = " << style::reset
              << fg::cyan   << square(3.14)       << style::reset << '\n';

    ok("concepts demo complete");
}

// ── main ──────────────────────────────────────────────────────────────────

int main(int argc, char* argv[]) {
    banner();

    if (argc > 1) {
        info(std::format("{} arg(s) received", argc - 1));
        for (int i = 1; i < argc; ++i)
            std::cout << "  " << fg::cyan << i << style::reset
                      << ": " << argv[i] << '\n';
    } else {
        warn("no arguments passed");
    }

    demo_ranges();
    demo_format();
    demo_concepts();

    std::cout << '\n';
    ok("all demos passed");
    std::cout << style::bold << fg::green << "done.\n" << style::reset;

    return 0;
}

#include "kaish/config.hpp"
#include "kaish/platform.hpp"
#include "kaish/shell.hpp"
#include "kaish/syntax.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace fs = std::filesystem;
using namespace kai::kaish;

// Unlike assert, these stay active in Release builds.
template <typename T>
std::string show(const T& v) {
    std::ostringstream ss;
    ss << v;
    return ss.str();
}

#define CHECK(expr)                                                                                   \
    do {                                                                                              \
        if (!(expr)) throw std::runtime_error("CHECK failed at line " + std::to_string(__LINE__) + ": " #expr); \
    } while (0)

#define CHECK_EQ(a, b)                                                                                \
    do {                                                                                              \
        const auto _a = (a);                                                                          \
        const auto _b = (b);                                                                          \
        if (!(_a == _b))                                                                              \
            throw std::runtime_error("CHECK_EQ failed at line " + std::to_string(__LINE__) + ": [" + \
                                     show(_a) + "] != [" + show(_b) + "]");                           \
    } while (0)

struct TempDir {
    fs::path path;
    TempDir() {
        path = fs::temp_directory_path() / ("kaish-test-" + std::to_string(platform::process_id()));
        fs::remove_all(path);
        fs::create_directories(path);
    }
    ~TempDir() {
        std::error_code ec;
        fs::remove_all(path, ec);
    }
    std::string str() const { return platform::from_path(path); }
    void touch(const std::string& name) const { std::ofstream(path / name) << name; }
};

// ------------------------------------------------------------------ syntax

void test_syntax_lists() {
    auto list = parse_command_line("echo a | grep b && ls; pwd");
    CHECK_EQ(list.size(), size_t{3});
    CHECK_EQ(list[0].pipeline.commands.size(), size_t{2});
    CHECK(list[0].next == Connector::And);
    CHECK(list[1].next == Connector::Seq);
}

void test_syntax_incomplete() {
    for (const char* src : {"echo 'abc", "ls |", "true &&", "echo \"x", "echo $(ls"}) {
        bool incomplete = false;
        try {
            parse_command_line(src);
        } catch (const SyntaxError& e) {
            incomplete = e.incomplete;
        }
        CHECK(incomplete);
    }
}

void test_windows_paths_survive() {
    auto list = parse_command_line("cd C:\\Users\\chris\\src");
    CHECK_EQ(list[0].pipeline.commands[0].words[1].raw(), std::string("C:\\Users\\chris\\src"));
    auto root = parse_command_line("cd C:\\");
    CHECK_EQ(root[0].pipeline.commands[0].words[1].raw(), std::string("C:\\"));
}

void test_redirect_parse() {
    auto list = parse_command_line("make 2>&1 > out.txt");
    const auto& cmd = list[0].pipeline.commands[0];
    CHECK_EQ(cmd.words.size(), size_t{1});
    CHECK_EQ(cmd.redirects.size(), size_t{2});
    CHECK(cmd.redirects[0].kind == Redirect::Kind::Dup);
    CHECK_EQ(cmd.redirects[1].target.raw(), std::string("out.txt"));
}

// -------------------------------------------------------------- execution

void test_expansion_and_quoting() {
    Shell sh;
    sh.set_var("X", "hi");
    CHECK_EQ(sh.capture("echo $X 'lit $X' \"q $X\" ${X}there"), std::string("hi lit $X q hi hithere\n"));
    CHECK_EQ(sh.capture("echo a $NOPE_NOT_SET b"), std::string("a b\n"));
    CHECK_EQ(sh.capture("echo x$(echo inner)y"), std::string("xinnery\n"));
}

void test_status_and_connectors() {
    Shell sh;
    CHECK_EQ(sh.capture("false; echo $?"), std::string("1\n"));
    CHECK_EQ(sh.capture("false && echo a || echo b"), std::string("b\n"));
    CHECK_EQ(sh.capture("true && echo yes"), std::string("yes\n"));
    sh.capture("no_such_command_kaish_xyz");
    CHECK_EQ(sh.status, 127);
}

void test_pipelines() {
    Shell sh;
    CHECK_EQ(sh.capture("echo -e 'b\\na\\nb' | sort -u"), std::string("a\nb\n"));
    CHECK_EQ(sh.capture("echo hello | wc -c"), std::string("6\n"));
    CHECK_EQ(sh.capture("echo -e 'apple\\nbanana\\ncherry' | grep an"), std::string("banana\n"));
    CHECK_EQ(sh.capture("echo -e '1\\n2\\n3' | head -n 2"), std::string("1\n2\n"));
    CHECK_EQ(sh.capture("echo -e '1\\n2\\n3' | tail -1"), std::string("3\n"));
}

void test_redirection() {
    TempDir dir;
    Shell sh;
    const std::string f = "'" + dir.str() + "/out.txt'";
    CHECK_EQ(sh.capture("echo one > " + f + "; echo two >> " + f + "; cat " + f), std::string("one\ntwo\n"));
    CHECK_EQ(sh.capture("cat < " + f + " | wc -l"), std::string("2\n"));
}

void test_ls_and_glob() {
    TempDir dir;
    dir.touch("b.txt");
    dir.touch("a.txt");
    dir.touch("c.log");
    dir.touch(".hidden");
    Shell sh;
    const std::string d = "'" + dir.str() + "'";
    CHECK_EQ(sh.capture("ls " + d), std::string("a.txt\nb.txt\nc.log\n"));
    CHECK_EQ(sh.capture("ls -A " + d + " | wc -l"), std::string("4\n"));
    CHECK_EQ(sh.capture("echo " + d + "/*.txt"), dir.str() + "/a.txt " + dir.str() + "/b.txt\n");
    CHECK_EQ(sh.capture("echo " + d + "/*.none"), dir.str() + "/*.none\n");
    CHECK_EQ(sh.capture("echo '*.txt'"), std::string("*.txt\n"));
}

void test_file_builtins() {
    TempDir dir;
    Shell sh;
    const std::string d = "'" + dir.str() + "'";
    sh.execute("mkdir -p " + d + "/x/y && touch " + d + "/x/y/f && cp -r " + d + "/x " + d + "/z");
    CHECK(fs::exists(dir.path / "z" / "y" / "f"));
    sh.execute("mv " + d + "/z " + d + "/w");
    CHECK(fs::exists(dir.path / "w" / "y" / "f"));
    sh.execute("rm -rf " + d + "/w " + d + "/x");
    CHECK(!fs::exists(dir.path / "w"));
    CHECK(!fs::exists(dir.path / "x"));
}

void test_cd_and_pwd() {
    TempDir dir;
    Shell sh;
    const auto before = fs::current_path();
    sh.execute("cd '" + dir.str() + "'");
    CHECK(fs::equivalent(fs::current_path(), dir.path));
    sh.execute("cd -");
    CHECK(fs::equivalent(fs::current_path(), before));
}

void test_alias_and_assign() {
    Shell sh;
    sh.execute("alias greet='echo hello'");
    CHECK_EQ(sh.capture("greet world"), std::string("hello world\n"));
    sh.execute("NAME=kaish");
    CHECK_EQ(sh.capture("echo $NAME"), std::string("kaish\n"));
}

// ------------------------------------------------------------------ stack

std::string stack_dump(Shell& sh) {
    std::string r;
    for (size_t level = sh.stack().depth(); level >= 1; --level) r += sh.stack().text(level) + "|";
    return r;
}

void test_ls_pushes_each_entry() {
    TempDir dir;
    dir.touch("b.txt");
    dir.touch("a.txt");
    dir.touch("c.log");
    Shell sh;
    sh.execute("ls '" + dir.str() + "'", true);
    CHECK_EQ(stack_dump(sh), std::string("a.txt|b.txt|c.log|"));
    CHECK_EQ(sh.stack().text(1), std::string("c.log"));
}

void test_text_output_is_one_entry() {
    TempDir dir;
    dir.touch("a.txt");
    dir.touch("b.txt");
    dir.touch("c.log");
    Shell sh;
    sh.execute("echo -e 'one\\ntwo'", true);
    CHECK_EQ(sh.stack().depth(), size_t{1});
    CHECK_EQ(sh.stack().text(1), std::string("one\ntwo"));
    sh.execute("ls '" + dir.str() + "' | grep txt", true);  // last stage of a pipe: one entry
    CHECK_EQ(sh.stack().depth(), size_t{2});
    CHECK_EQ(sh.stack().text(1), std::string("a.txt\nb.txt"));
}

void test_side_effects_push_nothing() {
    TempDir dir;
    Shell sh;
    const auto before = fs::current_path();
    sh.execute("cd '" + dir.str() + "'; mkdir sub; echo hi > f.txt; cd -", true);
    CHECK_EQ(sh.stack().depth(), size_t{1});   // only `cd -` prints the directory
    CHECK(fs::exists(dir.path / "f.txt"));
    fs::current_path(before);
}

void test_stack_words() {
    Shell sh;
    sh.execute("echo x; echo y", true);
    CHECK_EQ(stack_dump(sh), std::string("x|y|"));
    sh.execute("swap", true);
    CHECK_EQ(stack_dump(sh), std::string("y|x|"));
    sh.execute("dup", true);
    CHECK_EQ(stack_dump(sh), std::string("y|x|x|"));
    sh.execute("drop 2", true);
    CHECK_EQ(stack_dump(sh), std::string("y|"));
    sh.execute("clr", true);
    CHECK_EQ(sh.stack().depth(), size_t{0});
    sh.execute("drop", true);  // HP48: Too few arguments
    CHECK_EQ(sh.status, 1);
}

void test_term_prints_instead() {
    Shell sh;
    CHECK_EQ(sh.capture("term echo hi"), std::string("hi\n"));
    sh.execute("term echo hi > /dev/null", true);
    CHECK_EQ(sh.stack().depth(), size_t{0});
}

void test_render_stack() {
    Config c;
    c.stack_levels = 8;
    Shell sh(make_text_backend(), c);
    for (int i = 0; i < 10; ++i) sh.stack().push_text("v" + std::to_string(i));
    std::ostringstream out;
    sh.render_stack(out);
    const std::string r = out.str();
    CHECK(r.find("2 more") != std::string::npos);
    CHECK(r.find("8:") != std::string::npos);
    CHECK(r.find("9:") == std::string::npos);
    CHECK(r.find("\"v9\"") != std::string::npos);   // level 1 is the newest
    CHECK(r.find("\"v1\"") == std::string::npos);   // below the 8 shown levels
}

void test_modes_without_kai() {
    Shell sh;
    sh.dispatch("pi");
    CHECK(sh.mode == Mode::Ps);
    CHECK_EQ(sh.status, 1);
    sh.dispatch("echo still-ps");
    CHECK_EQ(sh.stack().text(1), std::string("still-ps"));
}

void test_stack_references() {
    Shell sh;
    sh.stack().push_text("a  b");   // two spaces: must stay one argument
    sh.stack().push_text("hello");
    sh.execute("echo @1 x@1 '@1' @2", true);
    CHECK_EQ(sh.stack().text(1), std::string("hello x@1 @1 a  b"));
    CHECK_EQ(sh.stack().depth(), size_t{3});   // references do not consume

    sh.execute("echo @9", true);
    CHECK_EQ(sh.status, 1);
    CHECK_EQ(sh.stack().depth(), size_t{3});   // command did not run

    TempDir dir;
    dir.touch("src.txt");
    sh.stack().push_text(dir.str() + "/src.txt");
    sh.stack().push_text(dir.str() + "/copy.txt");
    sh.execute("cp @2 @1", true);
    CHECK(fs::exists(dir.path / "copy.txt"));
}

void test_ps_with_arguments_is_a_command() {
    Shell sh;
    sh.aliases["ps"] = "echo ran";
    sh.dispatch("ps aux");
    CHECK(sh.mode == Mode::Ps);
    CHECK_EQ(sh.stack().text(1), std::string("ran aux"));
}

void test_kai_script_without_kai() {
    TempDir dir;
    { std::ofstream(dir.path / "t.pi") << "1 2 +\n"; }
    Shell sh;
    CHECK_EQ(sh.run_file(dir.str() + "/t.pi", {}), 1);
    CHECK(script_language("x.PI") == Mode::Pi);
    CHECK(script_language("x.rho") == Mode::Rho);
    CHECK(!script_language("x.sh"));
}

void test_config() {
    std::ostringstream err;
    Config c = parse_config(R"({"stack_levels": 4, "mode": "rho", "show_stack": false, "passthrough": ["vim", "less"], "other": 1})", err);
    CHECK_EQ(c.stack_levels, 4);
    CHECK(c.mode == Mode::Rho);
    CHECK(!c.show_stack);
    CHECK_EQ(c.passthrough.size(), size_t{2});
    CHECK(err.str().empty());

    std::ostringstream err2;
    Config bad = parse_config("{ \"stack_levels\": 0, ", err2);
    CHECK_EQ(bad.stack_levels, 8);
    CHECK(!err2.str().empty());

    std::ostringstream err3;
    Config round = parse_config(config_to_json(c), err3);
    CHECK_EQ(round.stack_levels, 4);
    CHECK(round.mode == Mode::Rho);
    CHECK(err3.str().empty());
}

int main() {
    std::cout << "Running kaish unit tests...\n";
    struct Test {
        const char* name;
        void (*fn)();
    };
    const Test tests[] = {
        {"syntax_lists", test_syntax_lists},
        {"syntax_incomplete", test_syntax_incomplete},
        {"windows_paths_survive", test_windows_paths_survive},
        {"redirect_parse", test_redirect_parse},
        {"expansion_and_quoting", test_expansion_and_quoting},
        {"status_and_connectors", test_status_and_connectors},
        {"pipelines", test_pipelines},
        {"redirection", test_redirection},
        {"ls_and_glob", test_ls_and_glob},
        {"file_builtins", test_file_builtins},
        {"cd_and_pwd", test_cd_and_pwd},
        {"alias_and_assign", test_alias_and_assign},
        {"ls_pushes_each_entry", test_ls_pushes_each_entry},
        {"text_output_is_one_entry", test_text_output_is_one_entry},
        {"side_effects_push_nothing", test_side_effects_push_nothing},
        {"stack_words", test_stack_words},
        {"term_prints_instead", test_term_prints_instead},
        {"render_stack", test_render_stack},
        {"modes_without_kai", test_modes_without_kai},
        {"stack_references", test_stack_references},
        {"ps_with_arguments_is_a_command", test_ps_with_arguments_is_a_command},
        {"kai_script_without_kai", test_kai_script_without_kai},
        {"config", test_config},
    };
    int failed = 0;
    for (const auto& t : tests) {
        try {
            t.fn();
            std::cout << "[PASSED] " << t.name << "\n";
        } catch (const std::exception& e) {
            std::cout << "[FAILED] " << t.name << ": " << e.what() << "\n";
            ++failed;
        }
    }
    if (failed) {
        std::cout << failed << " test(s) failed\n";
        return 1;
    }
    std::cout << "All tests passed successfully!\n";
    return 0;
}

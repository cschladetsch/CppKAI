#pragma once

#include "backend.hpp"
#include "config.hpp"
#include "syntax.hpp"

#include <functional>
#include <iosfwd>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace kai::kaish {

    struct Shell;

    // Streams a builtin reads and writes; pipes and redirections are already applied.
    // When `stack` is set the result is headed for the value stack: write text to
    // `out` to push it as one entry, or push items to `stack` directly (ls does).
    struct CommandIo {
        std::istream& in;
        std::ostream& out;
        std::ostream& err;
        bool out_is_terminal = false;
        ValueStack* stack = nullptr;
    };

    using Args = std::vector<std::string>;
    using Builtin = std::function<int(Shell&, const Args&, CommandIo&)>;

    bool is_valid_name(std::string_view s);

    // kaish: a bash-flavoured shell (ps mode) blended with KAI's Pi and Rho, all
    // sharing one value stack that is shown after every command.
    struct Shell {
        explicit Shell(std::unique_ptr<Backend> backend = make_text_backend(), Config config = {});

        // Runs ps-mode source. With to_stack, the final output of each pipeline is
        // pushed onto the stack instead of printed.
        int execute(std::string_view source, bool to_stack = false);
        std::string capture(std::string_view source);   // run and return stdout, like $(...)
        void dispatch(const std::string& line);          // one interactive line in the current mode
        void run_in(Mode m, const std::string& code);
        int run_interactive();
        int run_file(const std::string& path, const Args& args);   // .pi/.rho run as KAI
        int run_kai_file(Mode lang, const std::string& path);

        ValueStack& stack() { return backend->stack(); }
        void render_stack(std::ostream& out);

        std::optional<std::string> get_var(const std::string& name) const;
        void set_var(const std::string& name, const std::string& value);
        void export_var(const std::string& name, const std::string& value);
        void unset_var(const std::string& name);

        std::string prompt() const;
        std::string history_file() const;
        std::optional<std::string> expand_history(const std::string& line) const;

        std::unique_ptr<Backend> backend;
        Config config;
        Mode mode = Mode::Ps;
        std::map<std::string, Builtin> builtins;
        std::map<std::string, std::string> aliases;
        std::map<std::string, std::string> vars;   // shell (non-exported) variables
        std::vector<std::string> history;
        Args positional{"kaish"};                    // $0 $1 ...
        int status = 0;
        bool exiting = false;
        int exit_code = 0;

    private:
        struct StageIo;
        int run_list(const CommandList& list, const StageIo& io);
        int run_pipeline(const Pipeline& p, const StageIo& io);
        int run_command(const SimpleCommand& cmd, StageIo io);
        std::vector<std::string> expand(const Word& w);
        bool is_passthrough(const std::string& program) const;
        bool needs_more_input(const std::string& text);
        std::string temp_path();
        unsigned temp_counter_ = 0;
    };

    std::optional<Mode> script_language(const std::string& path);  // Pi for .pi, Rho for .rho
    void register_builtins(Shell& shell);

}

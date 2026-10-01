#include "kaish/config.hpp"
#include "kaish/platform.hpp"
#include "kaish/shell.hpp"

#include <iostream>
#include <sstream>

// kaish                 interactive (ps, pi and rho modes over one stack)
// kaish -c 'cmd' [args] run one command line
// kaish script [args]   run a script
// kaish < script        run commands from stdin
int main(int argc, char** argv) {
    using namespace kai::kaish;
    platform::init_console();
    const auto args = platform::command_line_args(argc, argv);

    if (args.size() >= 2 && (args[1] == "-h" || args[1] == "--help")) {
        std::cout << "usage: kaish [-i] [-c command [args...]] [script [args...]]\n";
        return 0;
    }

    const bool command_mode = args.size() >= 3 && args[1] == "-c";
    const bool script_mode = !command_mode && args.size() >= 2 && args[1] != "-i";
    const bool interactive = !command_mode && !script_mode &&
                             (platform::stdin_is_console() || (args.size() >= 2 && args[1] == "-i"));

    const std::string config_path = platform::from_path(platform::home_dir() / ".kaish.json");
    Config config = load_config(config_path, std::cerr, interactive);

#ifdef KAISH_WITH_KAI
    auto backend = make_kai_backend();
#else
    auto backend = make_text_backend();
#endif
    Shell shell(std::move(backend), config);
    auto result = [&] { return shell.exiting ? shell.exit_code : shell.status; };

    if (command_mode) {
        shell.positional.assign(args.begin() + 3, args.end());
        shell.positional.insert(shell.positional.begin(), "kaish");
        shell.execute(args[2]);
        return result();
    }
    if (script_mode) {
        const int rc = shell.run_file(args[1], Args(args.begin() + 2, args.end()));
        if (script_language(args[1])) shell.render_stack(std::cout);  // show what the script left
        return rc;
    }
    if (interactive) return shell.run_interactive();

    std::ostringstream script;
    script << std::cin.rdbuf();
    shell.execute(script.str());
    return result();
}

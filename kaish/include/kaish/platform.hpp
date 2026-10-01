#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <utility>
#include <vector>

// Thin OS layer. All strings are UTF-8; Windows code converts to UTF-16 at the boundary.
namespace kai::kaish::platform {

    namespace fs = std::filesystem;

    fs::path to_path(const std::string& utf8);
    std::string from_path(const fs::path& p);

    std::vector<std::string> command_line_args(int argc, char** argv);

    std::optional<std::string> get_env(const std::string& name);
    void set_env(const std::string& name, const std::string& value);
    void unset_env(const std::string& name);
    std::vector<std::pair<std::string, std::string>> environment();

    void init_console();      // UTF-8 code pages, ANSI escape sequences
    void ignore_interrupts(); // Ctrl-C goes to the child process, not the shell
    bool stdin_is_console();
    bool stdout_is_console();
    int terminal_width();

    // One line of interactive input. Returns false on EOF (Ctrl-D, or Ctrl-Z on Windows).
    // Ctrl-C yields an empty line.
    bool read_line(std::string& line);

    std::string user_name();
    std::string host_name();
    fs::path home_dir();
    int process_id();

    bool is_hidden(const fs::path& p, const std::string& name);
    bool is_executable(const fs::path& p);

    // PATH lookup (PATHEXT on Windows). Names containing a separator are resolved directly.
    std::optional<fs::path> find_executable(const std::string& name);

    struct StdioSpec {
        std::string in_path;   // empty = inherit
        std::string out_path;
        bool out_append = false;
        std::string err_path;
        bool err_append = false;
        bool err_to_out = false;
        bool out_to_err = false;
    };

    // Runs a program and waits for it. Returns its exit status, or -1 if it could not
    // be started. Throws std::runtime_error if a redirection file cannot be opened.
    int run_process(const fs::path& exe, const std::vector<std::string>& argv, const StdioSpec& io);

}

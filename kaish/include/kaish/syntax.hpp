#pragma once

#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace kai::kaish {

    // A word is a sequence of segments; quoting decides how each segment is expanded.
    struct Segment {
        enum class Quote { None, Single, Double };
        std::string text;
        Quote quote = Quote::None;
    };

    struct Word {
        std::vector<Segment> parts;
        bool is_plain() const;   // no quoting anywhere
        std::string raw() const; // text with quotes removed, no expansion
    };

    struct Redirect {
        enum class Kind { In, Out, Append, Dup, OutErr };
        int fd = 1;
        Kind kind = Kind::Out;
        Word target;   // unused for Dup
        int dup_fd = 1;
    };

    struct SimpleCommand {
        std::vector<Word> words;
        std::vector<Redirect> redirects;
    };

    struct Pipeline {
        std::vector<SimpleCommand> commands;
    };

    enum class Connector { Seq, And, Or };

    struct ListItem {
        Pipeline pipeline;
        Connector next = Connector::Seq;
    };

    using CommandList = std::vector<ListItem>;

    struct SyntaxError : std::runtime_error {
        bool incomplete; // more input would fix it (open quote, trailing |, &&, \)
        explicit SyntaxError(const std::string& msg, bool inc = false)
            : std::runtime_error(msg), incomplete(inc) {}
    };

    // Parses bash-style command lines: words, quotes, $(...), `...`, |, &&, ||, ;,
    // newlines, comments and redirections (<, >, >>, 2>, 2>>, 2>&1, 1>&2, &>).
    CommandList parse_command_line(std::string_view source);

}

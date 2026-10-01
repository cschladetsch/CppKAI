#pragma once

#include <cstddef>
#include <memory>
#include <string>

namespace kai::kaish {

    enum class Mode { Ps, Pi, Rho };
    const char* mode_name(Mode m);

    // The value stack every mode shares. Level 1 is the top, as on the HP48.
    // Operations on too few items throw std::runtime_error("Too few arguments").
    struct ValueStack {
        virtual ~ValueStack() = default;
        virtual std::size_t depth() const = 0;
        virtual bool is_text(std::size_t level) const = 0;
        virtual std::string text(std::size_t level) const = 0;  // the value as a plain string
        virtual std::string show(std::size_t level) const = 0;  // the value as displayed
        virtual void push_text(const std::string& s) = 0;
        virtual void drop() = 0;
        virtual void dup() = 0;
        virtual void swap() = 0;
        virtual void clear() = 0;
    };

    // Owns the stack and evaluates Pi and Rho. The KAI backend drives the real
    // Console/Executor; the text backend is a string stack with no languages,
    // used for standalone builds and tests.
    struct Backend {
        virtual ~Backend() = default;
        virtual ValueStack& stack() = 0;
        virtual bool has_languages() const = 0;
        virtual std::string eval(Mode mode, const std::string& code) = 0; // "" or an error message
        virtual bool incomplete(Mode mode, const std::string& code) = 0;  // e.g. an open Rho brace
    };

    std::unique_ptr<Backend> make_text_backend();
    std::unique_ptr<Backend> make_kai_backend(); // defined only when built inside CppKAI

}

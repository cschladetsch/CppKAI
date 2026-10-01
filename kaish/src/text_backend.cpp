#include "kaish/backend.hpp"

#include <stdexcept>
#include <utility>
#include <vector>

namespace kai::kaish {

    const char* mode_name(Mode m) {
        switch (m) {
            case Mode::Pi: return "pi";
            case Mode::Rho: return "rho";
            case Mode::Ps: break;
        }
        return "ps";
    }

    namespace {

        class TextStack final : public ValueStack {
        public:
            std::size_t depth() const override { return items_.size(); }
            bool is_text(std::size_t level) const override { at(level); return true; }
            std::string text(std::size_t level) const override { return at(level); }
            std::string show(std::size_t level) const override { return "\"" + at(level) + "\""; }
            void push_text(const std::string& s) override { items_.push_back(s); }
            void drop() override { need(1); items_.pop_back(); }
            void dup() override { need(1); items_.push_back(items_.back()); }
            void swap() override { need(2); std::swap(items_[items_.size() - 1], items_[items_.size() - 2]); }
            void clear() override { items_.clear(); }

        private:
            std::vector<std::string> items_;

            void need(std::size_t n) const {
                if (items_.size() < n) throw std::runtime_error("Too few arguments");
            }
            const std::string& at(std::size_t level) const {
                if (level == 0 || level > items_.size()) throw std::runtime_error("Too few arguments");
                return items_[items_.size() - level];
            }
        };

        class TextBackend final : public Backend {
        public:
            ValueStack& stack() override { return stack_; }
            bool has_languages() const override { return false; }
            std::string eval(Mode, const std::string&) override {
                return "kaish was built without KAI, so pi and rho are not available";
            }
            bool incomplete(Mode, const std::string&) override { return false; }

        private:
            TextStack stack_;
        };

    }

    std::unique_ptr<Backend> make_text_backend() { return std::make_unique<TextBackend>(); }

}

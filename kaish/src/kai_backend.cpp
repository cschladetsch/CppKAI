// The KAI backend: one KAI Console whose Executor data stack is the kaish stack.
// Pi and Rho lines go through Console::Process, so they behave exactly as they
// do in the KAI Console app. Compiled only when kaish is built inside CppKAI.

#include "kaish/backend.hpp"

#include <KAI/Console/Console.h>
#include <KAI/Core/Logger.h>

#include <stdexcept>

namespace kai::kaish {

    namespace {

        class KaiStack final : public ValueStack {
        public:
            explicit KaiStack(::kai::Console& console) : console_(console) {}

            std::size_t depth() const override {
                auto data = stack();
                return data.Exists() ? static_cast<std::size_t>(data->Size()) : 0;
            }

            bool is_text(std::size_t level) const override {
                return at(level).GetTypeNumber() == ::kai::Type::Number::String;
            }

            std::string text(std::size_t level) const override {
                const ::kai::Object obj = at(level);
                if (obj.GetTypeNumber() == ::kai::Type::Number::String) {
                    return ::kai::ConstDeref<::kai::String>(obj).StdString();
                }
                return obj.ToString().StdString();
            }

            std::string show(std::size_t level) const override {
                const ::kai::Object obj = at(level);
                if (obj.GetTypeNumber() == ::kai::Type::Number::String) {
                    return "\"" + ::kai::ConstDeref<::kai::String>(obj).StdString() + "\"";
                }
                return obj.ToString().StdString();
            }

            void push_text(const std::string& s) override {
                console_.GetExecutor()->Push(console_.GetRegistry().New<::kai::String>(::kai::String(s)));
            }

            void drop() override {
                need(1);
                stack()->Pop();
            }

            void dup() override {
                need(1);
                console_.GetExecutor()->Push(stack()->At(0));
            }

            void swap() override {
                need(2);
                auto data = stack();
                const ::kai::Object a = data->Pop();
                const ::kai::Object b = data->Pop();
                console_.GetExecutor()->Push(a);
                console_.GetExecutor()->Push(b);
            }

            void clear() override {
                auto data = stack();
                if (data.Exists()) data->Clear();
            }

        private:
            ::kai::Console& console_;

            ::kai::Value<::kai::Stack> stack() const { return console_.GetExecutor()->GetDataStack(); }

            void need(std::size_t n) const {
                if (depth() < n) throw std::runtime_error("Too few arguments");
            }

            ::kai::Object at(std::size_t level) const {
                need(level);
                if (level == 0) throw std::runtime_error("Too few arguments");
                return stack()->At(static_cast<int>(level - 1));
            }
        };

        class KaiBackend final : public Backend {
        public:
            KaiBackend() : stack_(console_) {}

            ValueStack& stack() override { return stack_; }
            bool has_languages() const override { return true; }

            std::string eval(Mode mode, const std::string& code) override {
                console_.SetLanguage(mode == Mode::Rho ? ::kai::Language::Rho : ::kai::Language::Pi);
                try {
                    return console_.Process(::kai::String(code)).StdString();
                } catch (const std::exception& e) {
                    return e.what();
                } catch (...) {
                    return "unknown error";
                }
            }

            bool incomplete(Mode mode, const std::string& code) override {
                console_.SetLanguage(mode == Mode::Rho ? ::kai::Language::Rho : ::kai::Language::Pi);
                return console_.IsStructureIncomplete(::kai::String(code));
            }

        private:
            ::kai::Console console_;
            KaiStack stack_;
        };

    }

    std::unique_ptr<Backend> make_kai_backend() {
        ::kai::Logger::Init();
        return std::make_unique<KaiBackend>();
    }

}

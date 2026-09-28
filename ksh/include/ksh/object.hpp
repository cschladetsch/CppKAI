#pragma once

#include <memory>
#include <string>
#include <vector>
#include <filesystem>

namespace kai::ksh {

    struct Context;

    struct Object : std::enable_shared_from_this<Object> {
        virtual ~Object() = default;
        virtual std::string to_string() const = 0;
        virtual bool is_executable() const { return false; }
        virtual void execute(Context&) {}
    };

    using ObjectPtr = std::shared_ptr<Object>;

    struct StringObject : public Object {
        std::string value;
        explicit StringObject(std::string val) : value(std::move(val)) {}
        std::string to_string() const override { return value; }
    };

    struct FileObject : public Object {
        std::filesystem::path path;
        explicit FileObject(std::filesystem::path p) : path(std::move(p)) {}
        std::string to_string() const override { return path.string(); }
    };

    struct CollectionObject : public Object {
        std::vector<ObjectPtr> items;
        std::string to_string() const override {
            std::string s = "[ ";
            for (const auto& item : items) {
                s += item->to_string() + " ";
            }
            return s + "]";
        }
    };

    struct CodeBlock : public Object {
        std::vector<ObjectPtr> instructions;
        bool is_executable() const override { return true; }
        void execute(Context& ctx) override;
        std::string to_string() const override;
    };

}

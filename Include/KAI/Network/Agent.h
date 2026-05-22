#pragma once

#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>

#include "KAI/Network/AgentBase.h"
#include "KAI/Network/Future.h"
#include "KAI/Network/NetHandle.h"

KAI_NET_BEGIN

template <class T>
struct Agent : AgentBase {
    using ServantPtr = std::shared_ptr<T>;

    explicit Agent(Node &node, ServantPtr servant = std::make_shared<T>())
        : AgentBase(node), servant_(std::move(servant)) {}

    T &Instance() { return *servant_; }
    const T &Instance() const { return *servant_; }

    NetHandle Handle() const { return this->GetHandle(); }

    template <typename R, typename... Args>
    void BindMethod(const std::string &name, R (T::*method)(Args...)) {
        auto bound = [servant = servant_, method](Args... args) -> R {
            return std::invoke(method, servant.get(),
                               std::forward<Args>(args)...);
        };
        this->GetNode().RegisterMethod(
            this->GetHandle(), name,
            std::function<R(Args...)>(std::move(bound)));
    }

    template <typename R, typename... Args>
    void BindMethod(const std::string &name, R (T::*method)(Args...) const) {
        auto bound = [servant = servant_, method](Args... args) -> R {
            return std::invoke(method, servant.get(),
                               std::forward<Args>(args)...);
        };
        this->GetNode().RegisterMethod(
            this->GetHandle(), name,
            std::function<R(Args...)>(std::move(bound)));
    }

    template <typename R, typename... Args>
    void BindMethod(const std::string &name, std::function<R(Args...)> fn) {
        this->GetNode().RegisterMethod(this->GetHandle(), name, std::move(fn));
    }

    template <typename Getter, typename Setter>
    void BindProperty(const std::string &name, Getter getter, Setter setter) {
        using ValueType =
            std::decay_t<decltype((std::declval<T &>().*getter)())>;

        auto getterFn = [servant = servant_, getter]() -> ValueType {
            return std::invoke(getter, servant.get());
        };

        auto setterFn = [servant = servant_, setter](ValueType value) {
            std::invoke(setter, servant.get(), value);
        };

        this->GetNode().RegisterProperty(
            this->GetHandle(), name, std::function<ValueType()>(getterFn),
            std::function<void(ValueType)>(setterFn));
    }

    template <typename Getter>
    void BindProperty(const std::string &name, Getter getter) {
        using ValueType =
            std::decay_t<decltype((std::declval<T &>().*getter)())>;

        auto getterFn = [servant = servant_, getter]() -> ValueType {
            return std::invoke(getter, servant.get());
        };

        this->GetNode().RegisterProperty(this->GetHandle(), name,
                                         std::function<ValueType()>(getterFn));
    }

    template <typename Member>
    void BindMemberProperty(const std::string &name, Member T::*member) {
        auto memberPtr = member;
        auto getter = [servant = servant_, memberPtr]() -> Member {
            return servant.get()->*memberPtr;
        };
        auto setter = [servant = servant_, memberPtr](Member value) {
            servant.get()->*memberPtr = value;
        };
        this->GetNode().RegisterProperty(this->GetHandle(), name,
                                         std::function<Member()>(getter),
                                         std::function<void(Member)>(setter));
    }

   private:
    ServantPtr servant_;
};

KAI_NET_END

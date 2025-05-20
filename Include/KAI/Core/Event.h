#pragma once

#include <concepts>
#include <functional>
#include <list>
#include <memory>
#include <utility>

#include "KAI/Core/BuiltinTypes/Void.h"

// DESCRIPTION
// This file contains a definition for a multi-cast Event using modern C++23
// features.
//
// An Event is a message sent by an object to signal
// the occurrence of an action. There may be many receivers
// for the event. Event receivers (also called Listeners or Observers)
// register their interest in an event by adding a delegate
// to the event:
//
// EXAMPLE
//        void foo(int, std::string);
//        struct Bar { void spam(int, std::string); };
//
//        Bar bar;
//        Event<int, std::string> event;            // create a new event which
//        takes int and string args event += foo;                             //
//        add a callback to foo function event += std::pair(&bar, &Bar::spam);
//        // add a callback to a bound method event(42, "hello"); // invoke the
//        event. foo and bar.spam will both be called event -= foo; // remove
//        foo function from delegate list event(12, ", world"); // now just
//        bar.spam will be called
//
// NOTES
// Return types for event delegates must return void.
// Now supports events with any number of arguments using variadic templates.
//
// REVISION
//        November 2007        @author christian        created
//        June 2008            @author christian        added SystemEvent
//        Feb 2017             @author christian        removed SystemEvent. I
//        forgot what it was for. May 2024             @author claude updated to
//        modern C++23

KAI_BEGIN

namespace event_detail {
// Concept to check if a type is callable with given arguments and returns void
template <typename F, typename... Args>
concept VoidCallable = requires(F f, Args... args) {
    { f(args...) } -> std::same_as<void>;
};

// Base delegate class for events
template <typename... Args>
struct Delegate {
    virtual ~Delegate() = default;
    virtual void invoke(Args... args) = 0;
};

// Function delegate implementation
template <typename... Args>
struct FunctionDelegate : public Delegate<Args...> {
    using FunctionType = std::function<void(Args...)>;

    FunctionDelegate(FunctionType func) : function(std::move(func)) {}

    void invoke(Args... args) override { function(args...); }

    bool equals(const FunctionType& other) const {
        // std::function doesn't support comparison in C++, so we use
        // target_type
        return function.target_type() == other.target_type() &&
               *function.template target<void (*)(Args...)>() ==
                   *other.template target<void (*)(Args...)>();
    }

    FunctionType function;
};

// Method delegate implementation
template <typename Class, typename... Args>
struct MethodDelegate : public Delegate<Args...> {
    using MethodType = void (Class::*)(Args...);

    MethodDelegate(Class* obj, MethodType method)
        : object(obj), method(method) {}

    void invoke(Args... args) override {
        (object->*method)(std::forward<Args>(args)...);
    }

    bool equals(Class* obj, MethodType meth) const {
        return object == obj && method == meth;
    }

    Class* object;
    MethodType method;
};

// Object-based method delegate
template <typename Class, typename... Args>
struct ObjectMethodDelegate : public Delegate<Args...> {
    using MethodType = void (Class::*)(Args...);

    ObjectMethodDelegate(Object obj, MethodType method)
        : object(obj), method(method) {}

    void invoke(Args... args) override {
        if (object.Exists()) {
            (Deref<Class>(object).*method)(std::forward<Args>(args)...);
        }
    }

    bool equals(const Object& obj, MethodType meth) const {
        return object == obj && method == meth;
    }

    Object object;
    MethodType method;
};

// Delegate type enum
enum class DelegateType { Function, Method, ObjectMethod };
}  // namespace event_detail

// Event implementation with variadic templates
template <typename... Args>
class Event {
   public:
    ~Event() { clear(); }

    // Function registration
    template <event_detail::VoidCallable<Args...> F>
    void operator+=(F&& function) {
        add(std::forward<F>(function));
    }

    // Function removal
    template <event_detail::VoidCallable<Args...> F>
    void operator-=(F&& function) {
        remove(std::forward<F>(function));
    }

    // Method registration with object pointer
    template <typename Class>
    void operator+=(const std::pair<Class*, void (Class::*)(Args...)>& bound) {
        addMethod(bound.first, bound.second);
    }

    // Method removal with object pointer
    template <typename Class>
    void operator-=(const std::pair<Class*, void (Class::*)(Args...)>& bound) {
        removeMethod(bound.first, bound.second);
    }

    // Method registration with Object
    template <typename Class>
    void operator+=(const std::pair<Object, void (Class::*)(Args...)>& bound) {
        addObjectMethod(bound.first, bound.second);
    }

    // Method removal with Object
    template <typename Class>
    void operator-=(const std::pair<Object, void (Class::*)(Args...)>& bound) {
        removeObjectMethod(bound.first, bound.second);
    }

    // Event invocation
    void operator()(Args... args) const {
        for (const auto& sink : sinks) {
            sink.second->invoke(std::forward<Args>(args)...);
        }
    }

    // Clear all delegates
    void clear() { sinks.clear(); }

   private:
    using DelegatePtr = std::shared_ptr<event_detail::Delegate<Args...>>;
    using SinkItem = std::pair<event_detail::DelegateType, DelegatePtr>;
    using SinkList = std::list<SinkItem>;

    SinkList sinks;

    // Add a function delegate
    template <typename F>
    void add(F&& function) {
        using FunctionDelegate = event_detail::FunctionDelegate<Args...>;
        sinks.emplace_back(
            event_detail::DelegateType::Function,
            std::make_shared<FunctionDelegate>(
                std::function<void(Args...)>(std::forward<F>(function))));
    }

    // Remove a function delegate
    template <typename F>
    void remove(F&& function) {
        std::function<void(Args...)> func = std::forward<F>(function);
        auto it = sinks.begin();
        while (it != sinks.end()) {
            if (it->first == event_detail::DelegateType::Function) {
                auto* functionDelegate =
                    static_cast<event_detail::FunctionDelegate<Args...>*>(
                        it->second.get());
                if (functionDelegate->equals(func)) {
                    it = sinks.erase(it);
                    return;
                }
            }
            ++it;
        }
    }

    // Add a method delegate
    template <typename Class>
    void addMethod(Class* object, void (Class::*method)(Args...)) {
        using MethodDelegate = event_detail::MethodDelegate<Class, Args...>;
        sinks.emplace_back(event_detail::DelegateType::Method,
                           std::make_shared<MethodDelegate>(object, method));
    }

    // Remove a method delegate
    template <typename Class>
    void removeMethod(Class* object, void (Class::*method)(Args...)) {
        auto it = sinks.begin();
        while (it != sinks.end()) {
            if (it->first == event_detail::DelegateType::Method) {
                auto* methodDelegate =
                    static_cast<event_detail::MethodDelegate<Class, Args...>*>(
                        it->second.get());
                if (methodDelegate->equals(object, method)) {
                    it = sinks.erase(it);
                    return;
                }
            }
            ++it;
        }
    }

    // Add an object method delegate
    template <typename Class>
    void addObjectMethod(const Object& object, void (Class::*method)(Args...)) {
        using ObjectMethodDelegate =
            event_detail::ObjectMethodDelegate<Class, Args...>;
        sinks.emplace_back(
            event_detail::DelegateType::ObjectMethod,
            std::make_shared<ObjectMethodDelegate>(object, method));
    }

    // Remove an object method delegate
    template <typename Class>
    void removeObjectMethod(const Object& object,
                            void (Class::*method)(Args...)) {
        auto it = sinks.begin();
        while (it != sinks.end()) {
            if (it->first == event_detail::DelegateType::ObjectMethod) {
                auto* objectMethodDelegate = static_cast<
                    event_detail::ObjectMethodDelegate<Class, Args...>*>(
                    it->second.get());
                if (objectMethodDelegate->equals(object, method)) {
                    it = sinks.erase(it);
                    return;
                }
            }
            ++it;
        }
    }
};

KAI_END
#pragma once

#include <KAI/Core/Object.h>

#include <functional>
#include <vector>

namespace kai {

// Simple implementation of Event for testing

// Event with no parameters
class Event0 {
   private:
    typedef std::function<void()> FunctionType;
    std::vector<FunctionType> functions;

   public:
    void Add(void (*func)()) { functions.push_back(FunctionType(func)); }

    template <class C>
    void AddMethod(C* instance, void (C::*method)()) {
        functions.push_back([instance, method]() { (instance->*method)(); });
    }

    template <class C>
    void AddMethod(Object obj, void (C::*method)()) {
        functions.push_back([obj, method]() {
            if (obj.Exists()) {
                (Deref<C>(obj).*method)();
            }
        });
    }

    void operator()() {
        for (auto& func : functions) {
            func();
        }
    }
};

// Event with one parameter
template <class T0>
class Event1 {
   private:
    typedef std::function<void(T0)> FunctionType;
    std::vector<FunctionType> functions;

   public:
    void Add(void (*func)(T0)) { functions.push_back(FunctionType(func)); }

    template <class C>
    void AddMethod(C* instance, void (C::*method)(T0)) {
        functions.push_back(
            [instance, method](T0 arg0) { (instance->*method)(arg0); });
    }

    template <class C>
    void AddMethod(Object obj, void (C::*method)(T0)) {
        functions.push_back([obj, method](T0 arg0) {
            if (obj.Exists()) {
                (Deref<C>(obj).*method)(arg0);
            }
        });
    }

    void operator()(T0 arg0) {
        for (auto& func : functions) {
            func(arg0);
        }
    }
};

// Event with two parameters
template <class T0, class T1>
class Event2 {
   private:
    typedef std::function<void(T0, T1)> FunctionType;
    std::vector<FunctionType> functions;

   public:
    void Add(void (*func)(T0, T1)) { functions.push_back(FunctionType(func)); }

    template <class C>
    void AddMethod(C* instance, void (C::*method)(T0, T1)) {
        functions.push_back([instance, method](T0 arg0, T1 arg1) {
            (instance->*method)(arg0, arg1);
        });
    }

    template <class C>
    void AddMethod(Object obj, void (C::*method)(T0, T1)) {
        functions.push_back([obj, method](T0 arg0, T1 arg1) {
            if (obj.Exists()) {
                (Deref<C>(obj).*method)(arg0, arg1);
            }
        });
    }

    void operator()(T0 arg0, T1 arg1) {
        for (auto& func : functions) {
            func(arg0, arg1);
        }
    }
};

// Event with three parameters
template <class T0, class T1, class T2>
class Event3 {
   private:
    typedef std::function<void(T0, T1, T2)> FunctionType;
    std::vector<FunctionType> functions;

   public:
    void Add(void (*func)(T0, T1, T2)) {
        functions.push_back(FunctionType(func));
    }

    template <class C>
    void AddMethod(C* instance, void (C::*method)(T0, T1, T2)) {
        functions.push_back([instance, method](T0 arg0, T1 arg1, T2 arg2) {
            (instance->*method)(arg0, arg1, arg2);
        });
    }

    template <class C>
    void AddMethod(Object obj, void (C::*method)(T0, T1, T2)) {
        functions.push_back([obj, method](T0 arg0, T1 arg1, T2 arg2) {
            if (obj.Exists()) {
                (Deref<C>(obj).*method)(arg0, arg1, arg2);
            }
        });
    }

    void operator()(T0 arg0, T1 arg1, T2 arg2) {
        for (auto& func : functions) {
            func(arg0, arg1, arg2);
        }
    }
};

// Template alias to simplify usage
template <class... Args>
struct Event;

template <>
struct Event<> {
    Event0 impl;
    void Add(void (*func)()) { impl.Add(func); }

    template <class C>
    void AddMethod(C* instance, void (C::*method)()) {
        impl.AddMethod(instance, method);
    }

    template <class C>
    void AddMethod(Object obj, void (C::*method)()) {
        impl.template AddMethod<C>(obj, method);
    }

    void operator()() { impl(); }
};

template <class T0>
struct Event<T0> {
    Event1<T0> impl;
    void Add(void (*func)(T0)) { impl.Add(func); }

    template <class C>
    void AddMethod(C* instance, void (C::*method)(T0)) {
        impl.AddMethod(instance, method);
    }

    template <class C>
    void AddMethod(Object obj, void (C::*method)(T0)) {
        impl.template AddMethod<C>(obj, method);
    }

    void operator()(T0 arg0) { impl(arg0); }
};

template <class T0, class T1>
struct Event<T0, T1> {
    Event2<T0, T1> impl;
    void Add(void (*func)(T0, T1)) { impl.Add(func); }

    template <class C>
    void AddMethod(C* instance, void (C::*method)(T0, T1)) {
        impl.AddMethod(instance, method);
    }

    template <class C>
    void AddMethod(Object obj, void (C::*method)(T0, T1)) {
        impl.template AddMethod<C>(obj, method);
    }

    void operator()(T0 arg0, T1 arg1) { impl(arg0, arg1); }
};

template <class T0, class T1, class T2>
struct Event<T0, T1, T2> {
    Event3<T0, T1, T2> impl;
    void Add(void (*func)(T0, T1, T2)) { impl.Add(func); }

    template <class C>
    void AddMethod(C* instance, void (C::*method)(T0, T1, T2)) {
        impl.AddMethod(instance, method);
    }

    template <class C>
    void AddMethod(Object obj, void (C::*method)(T0, T1, T2)) {
        impl.template AddMethod<C>(obj, method);
    }

    void operator()(T0 arg0, T1 arg1, T2 arg2) { impl(arg0, arg1, arg2); }
};

}  // namespace kai
#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include "KAI/Network/Config.h"
#include "KAI/Network/DateTime.h"
#include "KAI/Network/NetHandle.h"
#include "KAI/Network/ResponseType.h"

KAI_NET_BEGIN

// Shared-state Future to allow async completion without threads.
//
// Thread safety: State is guarded by a mutex, so it is safe for one thread
// to resolve a Future<T> (SetValue/SetResponse/SetComplete) while another
// thread observes it (IsComplete/Succeeded/GetValue/OnResolved) - see
// Test/Network/NodeFutureArgumentThreadTest.cpp for real cross-thread
// coverage of exactly this. It is still not a full condition-variable-style
// future: there is no blocking Wait(); an observer must poll IsComplete()
// or register an OnResolved() callback, same as before this class became
// internally synchronized.
//
// GetValue()/GetOptionalValue()/GetErrorMessage() intentionally return by
// value (a locked copy) rather than by reference, precisely so a caller
// can't be handed a reference into state that another thread might mutate
// out from under it a moment later.
template <class T = void>
struct Future {
    struct State {
        mutable std::mutex mutex;
        int id = 0;
        ResponseType response = ResponseType::None;
        bool complete = false;
        std::optional<T> value;
        std::string errorMessage;
        std::vector<std::function<void()>> onComplete;
    };

    Future() : state_(std::make_shared<State>()) {}

    [[nodiscard]] int GetId() const
    {
        std::lock_guard<std::mutex> lock(state_->mutex);
        return state_->id;
    }
    void SetId(int id) {
        std::lock_guard<std::mutex> lock(state_->mutex);
        state_->id = id;
    }

    [[nodiscard]] ResponseType GetResponse() const
    {
        std::lock_guard<std::mutex> lock(state_->mutex);
        return state_->response;
    }
    void SetResponse(ResponseType response) {
        std::lock_guard<std::mutex> lock(state_->mutex);
        state_->response = response;
    }

    [[nodiscard]] bool IsComplete() const
    {
        std::lock_guard<std::mutex> lock(state_->mutex);
        return state_->complete;
    }

    // Marking a future complete fires any callbacks registered via
    // OnResolved() exactly once, then clears them. There is still no
    // blocking wait here - completion is driven by whatever code calls
    // SetComplete(true), same as before; this just notifies observers when
    // that happens instead of requiring them to poll IsComplete(). The
    // callbacks are invoked OUTSIDE the lock (after being moved out of
    // State under it), so a callback that itself calls back into this
    // Future (e.g. to read GetValue()) cannot deadlock against Mutex, and a
    // slow callback cannot block a concurrent SetValue/IsComplete from
    // another thread.
    void SetComplete(bool complete) {
        std::vector<std::function<void()>> callbacks;
        {
            std::lock_guard<std::mutex> lock(state_->mutex);
            state_->complete = complete;
            if (complete && !state_->onComplete.empty()) {
                callbacks = std::move(state_->onComplete);
                state_->onComplete.clear();
            }
        }
        for (auto &cb : callbacks) {
            if (cb) {
                cb();
            }
        }
    }

    // Registers a callback to run once this future completes. If it has
    // already completed, the callback runs immediately (synchronously, but
    // outside the lock - see SetComplete's note on why).
    void OnResolved(std::function<void()> callback) {
        if (!callback) {
            return;
        }
        bool alreadyComplete = false;
        {
            std::lock_guard<std::mutex> lock(state_->mutex);
            if (state_->complete) {
                alreadyComplete = true;
            } else {
                state_->onComplete.push_back(std::move(callback));
                return;
            }
        }
        if (alreadyComplete) {
            callback();
        }
    }

    [[nodiscard]] std::optional<T> GetOptionalValue() const
    {
        std::lock_guard<std::mutex> lock(state_->mutex);
        return state_->value;
    }
    void SetValue(const T &value) {
        std::lock_guard<std::mutex> lock(state_->mutex);
        state_->value = value;
    }
    void SetValue(T &&value) {
        std::lock_guard<std::mutex> lock(state_->mutex);
        state_->value = std::move(value);
    }

    [[nodiscard]] std::string GetErrorMessage() const
    {
        std::lock_guard<std::mutex> lock(state_->mutex);
        return state_->errorMessage;
    }
    void SetErrorMessage(const std::string &message) {
        std::lock_guard<std::mutex> lock(state_->mutex);
        state_->errorMessage = message;
    }

    [[nodiscard]] bool Succeeded() const
    {
        std::lock_guard<std::mutex> lock(state_->mutex);
        return state_->complete && state_->response == ResponseType::Returned;
    }

    [[nodiscard]] T GetValue() const
    {
        std::lock_guard<std::mutex> lock(state_->mutex);
        if (!state_->value) {
            throw std::runtime_error("Future does not contain a value");
        }
        return *state_->value;
    }

    [[nodiscard]] std::shared_ptr<State> GetState() const
    {
        return state_;
    }

   private:
    std::shared_ptr<State> state_;
};

template <>
struct Future<void> {
    struct State {
        mutable std::mutex mutex;
        int id = 0;
        ResponseType response = ResponseType::None;
        bool complete = false;
        std::string errorMessage;
        std::vector<std::function<void()>> onComplete;
    };

    Future() : state_(std::make_shared<State>()) {}

    [[nodiscard]] int GetId() const
    {
        std::scoped_lock lock(state_->mutex);
        return state_->id;
    }
    void SetId(int id) {
        std::scoped_lock lock(state_->mutex);
        state_->id = id;
    }

    [[nodiscard]] ResponseType GetResponse() const
    {
        std::scoped_lock lock(state_->mutex);
        return state_->response;
    }
    void SetResponse(ResponseType response) {
        std::scoped_lock lock(state_->mutex);
        state_->response = response;
    }

    [[nodiscard]] bool IsComplete() const
    {
        std::scoped_lock lock(state_->mutex);
        return state_->complete;
    }

    void SetComplete(bool complete) {
        std::vector<std::function<void()>> callbacks;
        {
            std::scoped_lock lock(state_->mutex);
            state_->complete = complete;
            if (complete && !state_->onComplete.empty()) {
                callbacks = std::move(state_->onComplete);
                state_->onComplete.clear();
            }
        }
        for (auto &cb : callbacks) {
            if (cb) {
                cb();
            }
        }
    }

    void OnResolved(std::function<void()> callback) {
        if (!callback) {
            return;
        }
        bool alreadyComplete = false;
        {
            std::scoped_lock lock(state_->mutex);
            if (state_->complete) {
                alreadyComplete = true;
            } else {
                state_->onComplete.push_back(std::move(callback));
                return;
            }
        }
        if (alreadyComplete) {
            callback();
        }
    }

    [[nodiscard]] std::string GetErrorMessage() const
    {
        std::scoped_lock lock(state_->mutex);
        return state_->errorMessage;
    }
    void SetErrorMessage(const std::string &message) {
        std::scoped_lock lock(state_->mutex);
        state_->errorMessage = message;
    }

    [[nodiscard]] bool Succeeded() const
    {
        std::scoped_lock lock(state_->mutex);
        return state_->complete && state_->response == ResponseType::Returned;
    }

    [[nodiscard]] std::shared_ptr<State> GetState() const
    {
        return state_;
    }

   private:
    std::shared_ptr<State> state_;
};

// Type traits used by Node's argument-packing machinery to detect a
// Future<T> parameter/argument and recover its inner value type.
template <typename T> struct IsFuture : std::false_type {
};
template <typename T>
struct IsFuture<Future<T>> : std::true_type {};
template <typename T> inline constexpr bool kIsFutureV = IsFuture<std::decay_t<T>>::value;

template <typename T> struct FutureValue {
};
template <typename T>
struct FutureValue<Future<T>> {
    using type = T;
};
template <typename T> using future_value_t = FutureValue<std::decay_t<T>>::type;

KAI_NET_END

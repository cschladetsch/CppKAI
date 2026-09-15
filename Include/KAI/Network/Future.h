#pragma once

#include <functional>
#include <memory>
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
template <class T = void>
struct Future {
    struct State {
        int Id = 0;
        ResponseType Response = ResponseType::None;
        bool Complete = false;
        std::optional<T> Value;
        std::string ErrorMessage;
        std::vector<std::function<void()>> OnComplete;
    };

    Future() : state_(std::make_shared<State>()) {}

    int GetId() const { return state_->Id; }
    void SetId(int id) { state_->Id = id; }

    ResponseType GetResponse() const { return state_->Response; }
    void SetResponse(ResponseType response) { state_->Response = response; }

    bool IsComplete() const { return state_->Complete; }

    // Marking a future complete fires any callbacks registered via
    // OnResolved() exactly once, then clears them. There is still no
    // blocking wait here - completion is driven by whatever code calls
    // SetComplete(true), same as before; this just notifies observers when
    // that happens instead of requiring them to poll IsComplete().
    void SetComplete(bool complete) {
        state_->Complete = complete;
        if (complete && !state_->OnComplete.empty()) {
            auto callbacks = std::move(state_->OnComplete);
            state_->OnComplete.clear();
            for (auto &cb : callbacks) {
                if (cb) cb();
            }
        }
    }

    // Registers a callback to run once this future completes. If it has
    // already completed, the callback runs immediately (synchronously).
    void OnResolved(std::function<void()> callback) {
        if (!callback) return;
        if (state_->Complete) {
            callback();
        } else {
            state_->OnComplete.push_back(std::move(callback));
        }
    }

    const std::optional<T> &GetOptionalValue() const { return state_->Value; }
    void SetValue(const T &value) { state_->Value = value; }
    void SetValue(T &&value) { state_->Value = std::move(value); }

    const std::string &GetErrorMessage() const { return state_->ErrorMessage; }
    void SetErrorMessage(const std::string &message) {
        state_->ErrorMessage = message;
    }

    bool Succeeded() const {
        return state_->Complete && state_->Response == ResponseType::Returned;
    }

    const T &GetValue() const {
        if (!state_->Value) {
            throw std::runtime_error("Future does not contain a value");
        }
        return *state_->Value;
    }

    std::shared_ptr<State> GetState() const { return state_; }

   private:
    std::shared_ptr<State> state_;
};

template <>
struct Future<void> {
    struct State {
        int Id = 0;
        ResponseType Response = ResponseType::None;
        bool Complete = false;
        std::string ErrorMessage;
        std::vector<std::function<void()>> OnComplete;
    };

    Future() : state_(std::make_shared<State>()) {}

    int GetId() const { return state_->Id; }
    void SetId(int id) { state_->Id = id; }

    ResponseType GetResponse() const { return state_->Response; }
    void SetResponse(ResponseType response) { state_->Response = response; }

    bool IsComplete() const { return state_->Complete; }

    void SetComplete(bool complete) {
        state_->Complete = complete;
        if (complete && !state_->OnComplete.empty()) {
            auto callbacks = std::move(state_->OnComplete);
            state_->OnComplete.clear();
            for (auto &cb : callbacks) {
                if (cb) cb();
            }
        }
    }

    void OnResolved(std::function<void()> callback) {
        if (!callback) return;
        if (state_->Complete) {
            callback();
        } else {
            state_->OnComplete.push_back(std::move(callback));
        }
    }

    const std::string &GetErrorMessage() const { return state_->ErrorMessage; }
    void SetErrorMessage(const std::string &message) {
        state_->ErrorMessage = message;
    }

    bool Succeeded() const {
        return state_->Complete && state_->Response == ResponseType::Returned;
    }

    std::shared_ptr<State> GetState() const { return state_; }

   private:
    std::shared_ptr<State> state_;
};

// Type traits used by Node's argument-packing machinery to detect a
// Future<T> parameter/argument and recover its inner value type.
template <typename T>
struct is_future : std::false_type {};
template <typename T>
struct is_future<Future<T>> : std::true_type {};
template <typename T>
inline constexpr bool is_future_v = is_future<std::decay_t<T>>::value;

template <typename T>
struct future_value {};
template <typename T>
struct future_value<Future<T>> {
    using type = T;
};
template <typename T>
using future_value_t = typename future_value<std::decay_t<T>>::type;

KAI_NET_END

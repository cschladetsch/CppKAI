#pragma once

#include <memory>
#include <optional>
#include <stdexcept>
#include <string>

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
    };

    Future() : state_(std::make_shared<State>()) {}

    int GetId() const { return state_->Id; }
    void SetId(int id) { state_->Id = id; }

    ResponseType GetResponse() const { return state_->Response; }
    void SetResponse(ResponseType response) { state_->Response = response; }

    bool IsComplete() const { return state_->Complete; }
    void SetComplete(bool complete) { state_->Complete = complete; }

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
    };

    Future() : state_(std::make_shared<State>()) {}

    int GetId() const { return state_->Id; }
    void SetId(int id) { state_->Id = id; }

    ResponseType GetResponse() const { return state_->Response; }
    void SetResponse(ResponseType response) { state_->Response = response; }

    bool IsComplete() const { return state_->Complete; }
    void SetComplete(bool complete) { state_->Complete = complete; }

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

KAI_NET_END

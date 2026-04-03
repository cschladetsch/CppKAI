#pragma once

#include <KAI/Core/Config/Base.h>

#include "Event.h"

KAI_BEGIN

/// Base class for all timer objects that receive an Update() call.
class ProcessBase {
    bool paused;

   public:
    Event<Handle> OnUpdate;

    ProcessBase(bool P = true) : paused(P) {}
    virtual ~ProcessBase() {}

    bool IsPaused() const { return paused; }
    void Pause(bool P = true) { paused = P; }
    void Unpause() { paused = false; }

    virtual void Update(float time_delta) { KAI_UNUSED(time_delta); }
};

KAI_END

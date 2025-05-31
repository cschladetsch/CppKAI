#pragma once

#include <gtest/gtest.h>

#include "KAI/Core/BuiltinTypes.h"
#include "KAI/Core/Object/Class.h"
#include "KAI/Core/Object/GetStorageBase.h"

KAI_BEGIN

// Minimal test common class for core-only tests
class CoreTestCommon : public ::testing::Test {
   protected:
    Registry reg;
    Object root;

    void SetUp() override {
        // Add Map class first so we can create root
        reg.AddClass<Map>();

        AddRequiredClasses();

        // Create a root map to hold test objects
        root = reg.New<Map>();
    }

    virtual void AddRequiredClasses() {
        // Derived classes can override to add specific classes
    }

    Registry &Reg() { return reg; }
    const Registry &Reg() const { return reg; }
    Object &Root() { return root; }
    const Object &Root() const { return root; }
};

KAI_END
#include "TestCommon.h"

#include <KAI/Core/BuiltinTypes/All.h>
#include <KAI/Core/Tree.h>

#include "KAI/Core/Console.h"
#include "KAI/Core/StringStream.h"

KAI_BEGIN

void TestCommon::SetUp() {
    reg_ = new Registry();
    tree_ = new kai::Tree();

    reg_->AddClass<void>();
    reg_->AddClass<bool>();
    reg_->AddClass<int>();
    reg_->AddClass<float>();
    reg_->AddClass<String>();
    reg_->AddClass<StringStream>();
    reg_->AddClass<BinaryStream>();
    reg_->AddClass<Array>();
    reg_->AddClass<Map>();

    root_ = reg_->New<void>();
    tree_->SetRoot(root_);
    reg_->SetTree(*tree_);

    AddRequiredClasses();
}

Registry &TestCommon::Reg() const { return *reg_; }

Tree &TestCommon::GetTree() const { return *tree_; }

Object TestCommon::Root() const { return root_; }

void TestCommon::TearDown() {
    delete reg_;
    delete tree_;
}

KAI_END

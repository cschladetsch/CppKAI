// Additional TestCore coverage, following the same TestCommon fixture
// pattern used by TestRegistryOperations.cpp, TestArray.cpp and
// TestString.cpp: Registry create/retrieve for each built-in type, Array
// container edge cases, object identity/equality, and Registry-driven
// garbage collection lifecycle.

#include "TestCommon.h"

USING_NAMESPACE_KAI

struct CoreAdditionalTests2 : TestCommon {
   protected:
    void AddRequiredClasses() override {
        Reg().AddClass<bool>();
        Reg().AddClass<int>();
        Reg().AddClass<float>();
        Reg().AddClass<String>();
        Reg().AddClass<Array>();
    }
};

// --- Registry create/retrieve, across types and values ---

TEST_F(CoreAdditionalTests2, RegistryRoundTrip_IntPositive) {
    Pointer<int> obj = Reg().New<int>();
    *obj = 42;
    Root().Set(Label("rt_int_pos"), obj);

    Object retrieved = Root().Get(Label("rt_int_pos"));
    ASSERT_TRUE(retrieved.Exists());
    ASSERT_TRUE(retrieved.IsType<int>());
    ASSERT_EQ(ConstDeref<int>(retrieved), 42);
}

TEST_F(CoreAdditionalTests2, RegistryRoundTrip_IntNegative) {
    Pointer<int> obj = Reg().New<int>();
    *obj = -17;
    Root().Set(Label("rt_int_neg"), obj);

    Object retrieved = Root().Get(Label("rt_int_neg"));
    ASSERT_TRUE(retrieved.Exists());
    ASSERT_EQ(ConstDeref<int>(retrieved), -17);
}

TEST_F(CoreAdditionalTests2, RegistryRoundTrip_IntZero) {
    Pointer<int> obj = Reg().New<int>();
    *obj = 0;
    Root().Set(Label("rt_int_zero"), obj);

    Object retrieved = Root().Get(Label("rt_int_zero"));
    ASSERT_TRUE(retrieved.Exists());
    ASSERT_EQ(ConstDeref<int>(retrieved), 0);
}

TEST_F(CoreAdditionalTests2, RegistryRoundTrip_FloatPositive) {
    Pointer<float> obj = Reg().New<float>();
    *obj = 3.5f;
    Root().Set(Label("rt_float_pos"), obj);

    Object retrieved = Root().Get(Label("rt_float_pos"));
    ASSERT_TRUE(retrieved.Exists());
    ASSERT_TRUE(retrieved.IsType<float>());
    ASSERT_FLOAT_EQ(ConstDeref<float>(retrieved), 3.5f);
}

TEST_F(CoreAdditionalTests2, RegistryRoundTrip_FloatNegative) {
    Pointer<float> obj = Reg().New<float>();
    *obj = -2.25f;
    Root().Set(Label("rt_float_neg"), obj);

    Object retrieved = Root().Get(Label("rt_float_neg"));
    ASSERT_TRUE(retrieved.Exists());
    ASSERT_FLOAT_EQ(ConstDeref<float>(retrieved), -2.25f);
}

TEST_F(CoreAdditionalTests2, RegistryRoundTrip_BoolTrue) {
    Pointer<bool> obj = Reg().New<bool>();
    *obj = true;
    Root().Set(Label("rt_bool_true"), obj);

    Object retrieved = Root().Get(Label("rt_bool_true"));
    ASSERT_TRUE(retrieved.Exists());
    ASSERT_TRUE(retrieved.IsType<bool>());
    ASSERT_TRUE(ConstDeref<bool>(retrieved));
}

TEST_F(CoreAdditionalTests2, RegistryRoundTrip_BoolFalse) {
    Pointer<bool> obj = Reg().New<bool>();
    *obj = false;
    Root().Set(Label("rt_bool_false"), obj);

    Object retrieved = Root().Get(Label("rt_bool_false"));
    ASSERT_TRUE(retrieved.Exists());
    ASSERT_FALSE(ConstDeref<bool>(retrieved));
}

TEST_F(CoreAdditionalTests2, RegistryRoundTrip_StringNonEmpty) {
    Pointer<String> obj = Reg().New<String>("hello registry");
    Root().Set(Label("rt_str_nonempty"), obj);

    Object retrieved = Root().Get(Label("rt_str_nonempty"));
    ASSERT_TRUE(retrieved.Exists());
    ASSERT_TRUE(retrieved.IsType<String>());
    ASSERT_EQ(ConstDeref<String>(retrieved), "hello registry");
}

TEST_F(CoreAdditionalTests2, RegistryRoundTrip_StringEmpty) {
    Pointer<String> obj = Reg().New<String>("");
    Root().Set(Label("rt_str_empty"), obj);

    Object retrieved = Root().Get(Label("rt_str_empty"));
    ASSERT_TRUE(retrieved.Exists());
    ASSERT_EQ(ConstDeref<String>(retrieved), "");
}

TEST_F(CoreAdditionalTests2, RegistryOverwrite_SameLabelDifferentValue) {
    Root().Set(Label("rt_overwrite"), Reg().New<int>(1));
    Root().Set(Label("rt_overwrite"), Reg().New<int>(2));

    Object retrieved = Root().Get(Label("rt_overwrite"));
    ASSERT_TRUE(retrieved.Exists());
    ASSERT_EQ(ConstDeref<int>(retrieved), 2);
}

// --- Array container edge cases ---

TEST_F(CoreAdditionalTests2, Array_EmptyOnCreation) {
    Pointer<Array> array = Reg().New<Array>();
    Root().Set(Label("arr_empty"), array);

    ASSERT_TRUE(array->Empty());
    ASSERT_EQ(array->Size(), 0);
}

TEST_F(CoreAdditionalTests2, Array_SingleElement) {
    Pointer<Array> array = Reg().New<Array>();
    Root().Set(Label("arr_single"), array);

    array->PushBack(Reg().New<int>(7));

    ASSERT_EQ(array->Size(), 1);
    ASSERT_FALSE(array->Empty());
    ASSERT_EQ(ConstDeref<int>(array->At(0)), 7);
}

TEST_F(CoreAdditionalTests2, Array_MultipleIntElements) {
    Pointer<Array> array = Reg().New<Array>();
    Root().Set(Label("arr_multi"), array);

    for (int i = 0; i < 5; ++i) {
        array->PushBack(Reg().New<int>(i * 10));
    }

    ASSERT_EQ(array->Size(), 5);
    for (int i = 0; i < 5; ++i) {
        ASSERT_EQ(ConstDeref<int>(array->At(i)), i * 10);
    }
}

TEST_F(CoreAdditionalTests2, Array_ClearResetsSizeAndEmpty) {
    Pointer<Array> array = Reg().New<Array>();
    Root().Set(Label("arr_clear"), array);

    array->PushBack(Reg().New<int>(1));
    array->PushBack(Reg().New<int>(2));
    ASSERT_EQ(array->Size(), 2);

    array->Clear();
    ASSERT_TRUE(array->Empty());
    ASSERT_EQ(array->Size(), 0);
}

TEST_F(CoreAdditionalTests2, Array_MixedTypeElements) {
    Pointer<Array> array = Reg().New<Array>();
    Root().Set(Label("arr_mixed"), array);

    Object intElem = Reg().New<int>(1);
    Object strElem = Reg().New<String>("two");
    Object boolElem = Reg().New<bool>(true);
    Root().Set(Label("arr_mixed_int"), intElem);
    Root().Set(Label("arr_mixed_str"), strElem);
    Root().Set(Label("arr_mixed_bool"), boolElem);

    array->PushBack(intElem);
    array->PushBack(strElem);
    array->PushBack(boolElem);

    ASSERT_EQ(array->Size(), 3);
    ASSERT_TRUE(array->At(0).IsType<int>());
    ASSERT_TRUE(array->At(1).IsType<String>());
    ASSERT_TRUE(array->At(2).IsType<bool>());
    ASSERT_EQ(ConstDeref<int>(array->At(0)), 1);
    ASSERT_EQ(ConstDeref<String>(array->At(1)), "two");
    ASSERT_TRUE(ConstDeref<bool>(array->At(2)));
}

TEST_F(CoreAdditionalTests2, Array_NestedArrays) {
    Pointer<Array> outer = Reg().New<Array>();
    Root().Set(Label("arr_outer"), outer);

    Pointer<Array> inner = Reg().New<Array>();
    Root().Set(Label("arr_inner"), inner);
    inner->PushBack(Reg().New<int>(99));

    outer->PushBack(inner);

    ASSERT_EQ(outer->Size(), 1);
    Object retrievedInner = outer->At(0);
    ASSERT_TRUE(retrievedInner.IsType<Array>());
    ASSERT_EQ(ConstDeref<Array>(retrievedInner).Size(), 1);
    ASSERT_EQ(ConstDeref<int>(ConstDeref<Array>(retrievedInner).At(0)), 99);
}

TEST_F(CoreAdditionalTests2, Array_RepeatedPushBackGrowsSizeMonotonically) {
    Pointer<Array> array = Reg().New<Array>();
    Root().Set(Label("arr_grow"), array);

    for (int i = 1; i <= 10; ++i) {
        array->PushBack(Reg().New<int>(i));
        ASSERT_EQ(array->Size(), static_cast<size_t>(i));
    }
}

// --- Object identity and equality ---

TEST_F(CoreAdditionalTests2, ObjectIdentity_CopyShareHandle) {
    Object original = Reg().New(5);
    Root().Set(Label("id_original"), original);

    Object copy = original;
    ASSERT_EQ(original.GetHandle(), copy.GetHandle());
}

TEST_F(CoreAdditionalTests2, ObjectIdentity_IndependentObjectsDifferentHandles) {
    Object a = Reg().New(5);
    Object b = Reg().New(5);
    Root().Set(Label("id_a"), a);
    Root().Set(Label("id_b"), b);

    ASSERT_NE(a.GetHandle(), b.GetHandle());
}

TEST_F(CoreAdditionalTests2, ObjectEquality_SameValueEqual) {
    Object a = Reg().New(10);
    Object b = Reg().New(10);
    Root().Set(Label("eq_a"), a);
    Root().Set(Label("eq_b"), b);

    ASSERT_TRUE(a == b);
}

TEST_F(CoreAdditionalTests2, ObjectEquality_DifferentValueNotEqual) {
    Object a = Reg().New(10);
    Object b = Reg().New(20);
    Root().Set(Label("eq_c"), a);
    Root().Set(Label("eq_d"), b);

    ASSERT_TRUE(a != b);
}

TEST_F(CoreAdditionalTests2, ObjectExists_TrueAfterCreation) {
    Object obj = Reg().New(1);
    Root().Set(Label("exists_1"), obj);
    ASSERT_TRUE(obj.Exists());
}

TEST_F(CoreAdditionalTests2, ObjectExists_FalseForDefaultConstructed) {
    Object obj;
    ASSERT_FALSE(obj.Exists());
}

// --- Garbage collection lifecycle ---

TEST_F(CoreAdditionalTests2, GarbageCollection_RemovedObjectIsCollected) {
    {
        Pointer<int> obj = Reg().New<int>();
        *obj = 55;
        Root().Set(Label("gc_temp"), obj);

        Object retrieved = Root().Get(Label("gc_temp"));
        ASSERT_TRUE(retrieved.Exists());

        Root().Remove(Label("gc_temp"));
    }

    Reg().GarbageCollect();

    Object shouldNotExist = Root().Get(Label("gc_temp"));
    ASSERT_FALSE(shouldNotExist.Exists());
}

TEST_F(CoreAdditionalTests2, GarbageCollection_KeptObjectSurvives) {
    Pointer<int> obj = Reg().New<int>();
    *obj = 77;
    Root().Set(Label("gc_kept"), obj);

    Reg().GarbageCollect();

    Object retrieved = Root().Get(Label("gc_kept"));
    ASSERT_TRUE(retrieved.Exists());
    ASSERT_EQ(ConstDeref<int>(retrieved), 77);
}

TEST_F(CoreAdditionalTests2, GarbageCollection_MultipleObjectsIndependentLifetime) {
    Root().Set(Label("gc_multi_keep"), Reg().New<int>(1));
    {
        Root().Set(Label("gc_multi_temp"), Reg().New<int>(2));
        Root().Remove(Label("gc_multi_temp"));
    }

    Reg().GarbageCollect();

    ASSERT_TRUE(Root().Get(Label("gc_multi_keep")).Exists());
    ASSERT_FALSE(Root().Get(Label("gc_multi_temp")).Exists());
}

// --- String comparison via the class-level Equiv/Less/Greater operations,
//     following the pattern established in TestString.cpp ---

TEST_F(CoreAdditionalTests2, StringClassOps_EquivForEqualStrings) {
    Pointer<String> s0 = Reg().New<String>("match");
    Pointer<String> s1 = Reg().New<String>("match");
    Root().Set(Label("str_eq_a"), s0);
    Root().Set(Label("str_eq_b"), s1);

    ASSERT_TRUE(s0.GetClass()->Equiv(s0.GetStorageBase(), s1.GetStorageBase()));
}

TEST_F(CoreAdditionalTests2, StringClassOps_NotEquivForDifferentStrings) {
    Pointer<String> s0 = Reg().New<String>("aaa");
    Pointer<String> s1 = Reg().New<String>("bbb");
    Root().Set(Label("str_ne_a"), s0);
    Root().Set(Label("str_ne_b"), s1);

    ASSERT_FALSE(s0.GetClass()->Equiv(s0.GetStorageBase(), s1.GetStorageBase()));
}

TEST_F(CoreAdditionalTests2, StringClassOps_LessForLexicallySmaller) {
    Pointer<String> s0 = Reg().New<String>("aaa");
    Pointer<String> s1 = Reg().New<String>("bbb");
    Root().Set(Label("str_lt_a"), s0);
    Root().Set(Label("str_lt_b"), s1);

    ASSERT_TRUE(s0.GetClass()->Less(s0.GetStorageBase(), s1.GetStorageBase()));
    ASSERT_FALSE(
        s0.GetClass()->Greater(s0.GetStorageBase(), s1.GetStorageBase()));
}

TEST_F(CoreAdditionalTests2, StringClassOps_GreaterForLexicallyLarger) {
    Pointer<String> s0 = Reg().New<String>("zzz");
    Pointer<String> s1 = Reg().New<String>("aaa");
    Root().Set(Label("str_gt_a"), s0);
    Root().Set(Label("str_gt_b"), s1);

    ASSERT_TRUE(
        s0.GetClass()->Greater(s0.GetStorageBase(), s1.GetStorageBase()));
    ASSERT_FALSE(s0.GetClass()->Less(s0.GetStorageBase(), s1.GetStorageBase()));
}

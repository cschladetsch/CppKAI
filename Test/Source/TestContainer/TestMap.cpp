#include "KAI/Core/StringStream.h"
#include "TestCommon.h"

USING_NAMESPACE_KAI

class TestMap : public TestCommon {
   protected:
    void AddRequiredClasses() override {
        Reg().AddClass<Map>();
        Reg().AddClass<StringStream>();
    }
};

TEST_F(TestMap, TestCreation) {
    Pointer<Map> map = Reg().New<Map>();
    ASSERT_TRUE(map.Exists());
    ASSERT_TRUE(map->Size() == 0);
    ASSERT_TRUE(map->Empty());

    Reg().GarbageCollect();

    ASSERT_FALSE(map.Exists());
}

TEST_F(TestMap, TestInsertDelete) {
    Pointer<Map> map = Reg().New<Map>();
    Pointer<Map> dangling = Reg().New<Map>();

    // add the map to the root_ of the object tree_, so it can be
    // found and hence not GC'd
    root_.Set("map", map);
    Reg().GarbageCollect();

    // the map will still exist after the GC - but the `dangling` map
    // won't because it can't be reached by the Registry reg
    ASSERT_TRUE(map.Exists());
    ASSERT_FALSE(dangling.Exists());

    // make a key and a value to insert into map
    Pointer<int> n = Reg().New(42);
    Pointer<String> s = Reg().New<String>("Hello");

    map->Insert(n, s);

    ASSERT_TRUE(map->ContainsKey(n));
    Object found = map->GetValue(n);

    ASSERT_STREQ(ConstDeref<String>(found).c_str(), "Hello");
    ASSERT_EQ(found.GetHandle(), s.GetHandle());

    ASSERT_TRUE(map.Exists());
    ASSERT_TRUE(n.Exists());
    ASSERT_TRUE(found.Exists());

    // by removing the key associated with n, we also remove the value
    map->Erase(n);
    Reg().GarbageCollect();

    // now, neither the key nor value should exist,
    // but the map itself should exist because it was added
    // to the root_ of the tree_ above
    ASSERT_TRUE(map.Exists());
    ASSERT_FALSE(n.Exists());
    ASSERT_FALSE(found.Exists());
}

TEST_F(TestMap, TestComparison) {
    Pointer<Map> m0 = Reg().New<Map>();
    Pointer<Map> m1 = Reg().New<Map>();

    Object n = Reg().New(42);
    Pointer<String> s0 = Reg().New<String>("World");
    Pointer<String> s1 = Reg().New<String>("World");

    // make two value-identical maps (but with different value objects)
    m0->Insert(n, s0);
    m1->Insert(n, s1);

    // get a reference to the class for maps to make things easier to type
    ClassBase const &k = *m0.GetClass();

    // test that both maps are the same size and have same set of key/value
    // pairs (by object value)
    ASSERT_TRUE(k.Equiv(m0, m1));

    // change the value that the second map has for it's key valued 42
    *s1 = "Hello";
    ASSERT_FALSE(k.Equiv(m0, m1));

    // and just to prove that wasn't a fluke:
    *s1 = "World";
    ASSERT_TRUE(k.Equiv(m0, m1));
}

TEST_F(TestMap, TestStringStream) {
    Pointer<Map> m = Reg().New<Map>();
    Pointer<int> n = Reg().New(42);
    Pointer<String> s = Reg().New<String>("Hello");

    m->Insert(n, s);

    // make a new string stream, insert the map into it
    Pointer<StringStream> t = Reg().New<StringStream>();
    *t << m;

    std::cout << t->ToString().c_str() << std::endl;

    // here, we extract a map back out of the stream.
    //
    // not happy about this approach. we have to make the map with a Reg().stry,
    // before extracting it from a StringStream. It seems we should be able to
    // just say:
    //        Object q;
    //        *t >> q;
    //        ASSERT_TRUE(q.GetClass().GetType() == Type::Traits<Map>::Number);
    // Pointer<Map> m1 = Reg().New<Map>();
    // *t >> m1;

    // ensure the extracted map is the same value as the original map
    // ASSERT_EQ(*m, *m1);
}

TEST_F(TestMap, TestBinaryStream) {
    Pointer<Map> m0 = Reg().New<Map>();
    Pointer<Map> m1 = Reg().New<Map>();
    Pointer<int> n = Reg().New(42);
    Pointer<String> s = Reg().New<String>("Hello");

    m0->Insert(n, s);

    // make a new string stream, insert the map into it
    Pointer<BinaryStream> t = Reg().New<BinaryStream>();
    *t << m0;

    // TODO: BinaryStream  derives from BinaryPacket, which needs a Registry.
    // now, the BinaryStream has one, but there's currently no means to pass
    // that to the underlying BinaryPacket...
    // this means that the following fails, as it attempts to extract from
    // a BinaryPacket with a null Registry.
    //
    // There should be a way to inject custom code on construction. If it
    // doesn't exist in the current_ Traits system, it has to be added.
    //
    // The main functionality is to be able to pass arguments to base class
    // on construction...
    // *t >> m1;

    // ASSERT_EQ(*m0, *m1);
}

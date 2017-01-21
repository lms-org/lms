#include "gtest/gtest.h"
#include "lms/datamanager.h"
#include "lms/inheritance.h"
#include <iostream>

struct ChannelType : public lms::Inheritance {
    std::string key;
    int val;

    bool isSubType(std::type_index type) const {
        // TODO base type needs Inheritance for getWithType
        return false;
    }
};

struct DerivedChannelType : public ChannelType {
    int extra;

    bool isSubType(std::type_index type) const {
        return lms::Extends<ChannelType>::isSubType(this, type);
    }
};

struct SerialChannelType : public lms::Serializable {
    int i;

    void lmsSerialize(std::ostream &os,Type type) const override {
        (void)type;
        os << i;
    }

    virtual void lmsDeserialize(std::istream &is,Type type) override {
        (void)type;
        is >> i;
    }
};

TEST(DataManager, intType) {
    lms::DataManager dm;
    auto chWrite = dm.writeChannel<int>("Int");
    auto chRead = dm.readChannel<int>("Int");

    *chWrite = 10;

    ASSERT_EQ(10, *chRead);
}

TEST(DataManager, boolType) {
    lms::DataManager dm;
    auto chWrite = dm.writeChannel<bool>("Bool");
    auto chRead = dm.readChannel<bool>("Bool");

    *chWrite = true;
    ASSERT_TRUE(*chRead);

    *chWrite = false;
    ASSERT_FALSE(*chRead);
}

TEST(DataManager, writeAndReadChannel) {
    lms::DataManager dm;
    auto chWrite = dm.writeChannel<ChannelType>("A");
    auto chRead = dm.readChannel<ChannelType>("A");

    chWrite->key = "9000";
    chWrite->val = 42;

    ASSERT_EQ(std::string("9000"), chRead->key);
    ASSERT_EQ(42, chRead->val);
}

TEST(DataManager, inheritance_pointer) {
    DerivedChannelType derived;
    derived.key = "test";
    derived.val = 42;

    lms::Inheritance* inh = &derived;
    ChannelType* base = dynamic_cast<ChannelType*>(inh);

    ASSERT_EQ(std::string("test"), base->key);
    ASSERT_EQ(42, base->val);
}

TEST(DataManager, get) {
    lms::DataManager dm;
    auto ch = dm.writeChannel<int>("Int");

    ASSERT_EQ(ch.get(), &(*ch));
}

TEST(DataManager, castableTo) {
    lms::DataManager dm;
    auto ch = dm.writeChannel<DerivedChannelType>("Struct");
    ASSERT_TRUE(ch.castableTo<ChannelType>());
}

TEST(DataManager, getWithType) {
    lms::DataManager dm;
    auto ch = dm.writeChannel<DerivedChannelType>("Struct");
    ASSERT_TRUE(ch.castableTo<ChannelType>());
    ChannelType *ct = ch.getWithType<ChannelType>();

    ct->key = "hallo";
    ch->val = 42;

    ASSERT_EQ(42, ct->val);
    ASSERT_EQ(std::string("hallo"), ch->key);
}

TEST(DataManager, serialize) {
    lms::DataManager dm;
    std::stringstream ss;

    auto chOut = dm.writeChannel<SerialChannelType>("Out");
    chOut->i = 1234;
    chOut.serialize(ss);

    auto chIn = dm.writeChannel<SerialChannelType>("In");
    chIn.deserialize(ss);
    ASSERT_EQ(1234, chIn->i);
}

TEST(DataManager, inheritance) {
    lms::DataManager dm;
    auto chWrite = dm.writeChannel<ChannelType>("A");
    chWrite->key = "hallo";

    auto chWriteDerived = dm.writeChannel<DerivedChannelType>("A");

    ASSERT_EQ(std::string(""), chWriteDerived->key);
    ASSERT_EQ(std::string(""), chWrite->key);

    chWriteDerived->key = "test";
    chWriteDerived->val = 42;
    chWriteDerived->extra = 1;

    ASSERT_EQ(std::string("test"), chWriteDerived->key);

    ASSERT_EQ(std::string("test"), chWrite->key);
    ASSERT_EQ(42, chWrite->val);
}

TEST(DataManager, publishAndHasNewData) {
    lms::DataManager dm;
    auto chWrite = dm.writeChannel<int>("A");
    auto chRead1 = dm.readChannel<int>("A");
    auto chRead2 = dm.readChannel<int>("A");

    // initially there is no new data
    ASSERT_FALSE(chRead1.hasNewData());
    ASSERT_FALSE(chRead2.hasNewData());

    // now publish something
    chWrite.publish();

    // now there should be new data
    ASSERT_TRUE(chRead1.hasNewData());
    ASSERT_TRUE(chRead2.hasNewData());

    // But not a second time
    ASSERT_FALSE(chRead1.hasNewData());
    ASSERT_FALSE(chRead2.hasNewData());

    // publish a second time
    chWrite.publish();

    // now there should be again new data
    ASSERT_TRUE(chRead1.hasNewData());
    ASSERT_TRUE(chRead2.hasNewData());
}

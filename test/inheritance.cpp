#include "gtest/gtest.h"
#include "lms/inheritance.h"

struct Base {
    virtual ~Base() {}
};

struct OneDerived : public Base, public lms::Inheritance {
    bool isSubType(std::type_index type) const {
        return lms::Extends<Base>::isSubType(this, type);
    }
};

struct OtherDerived : public Base, public lms::Inheritance {
    bool isSubType(std::type_index type) const {
        return lms::Extends<Base>::isSubType(this, type);
    }
};

struct Leaf : public OneDerived {
    bool isSubType(std::type_index type) const {
        return lms::Extends<OneDerived>::isSubType(this, type);
    }
};

struct Diamond : public OneDerived, public OtherDerived {
    bool isSubType(std::type_index type) const {
        return lms::Extends<OneDerived, OtherDerived>::isSubType(this, type);
    }
};

TEST(Inheritance, isSubType) {
    OneDerived d1;
    EXPECT_TRUE(d1.isSubType(typeid(Base)));
    EXPECT_FALSE(d1.isSubType(typeid(OtherDerived)));
}

TEST(Inheritance, isSubType_deepTree) {
    Leaf leaf;
    EXPECT_TRUE(leaf.isSubType(typeid(OneDerived)));
    EXPECT_TRUE(leaf.isSubType(typeid(Base)));
    EXPECT_FALSE(leaf.isSubType(typeid(OtherDerived)));
}

TEST(Inheritance, isSubType_diamond) {
    Diamond diamond;
    EXPECT_TRUE(diamond.isSubType(typeid(OneDerived)));
    EXPECT_TRUE(diamond.isSubType(typeid(OtherDerived)));
    EXPECT_TRUE(diamond.isSubType(typeid(Base)));
    EXPECT_FALSE(diamond.isSubType(typeid(Leaf)));
}

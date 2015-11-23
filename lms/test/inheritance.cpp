#include "gtest/gtest.h"
#include "lms/inheritance.h"

struct Base {
    virtual ~Base() {}
};

struct OneDerived : public Base, public lms::Inheritance {
    bool isSubType(size_t hashcode) const {
        return lms::Impl<Base>::isSubType(hashcode, this);
    }
};

struct OtherDerived : public Base, public lms::Inheritance {
    bool isSubType(size_t hashcode) const {
        return lms::Impl<Base>::isSubType(hashcode, this);
    }
};

TEST(Inheritance, isSubType) {
    OneDerived d1;
    EXPECT_TRUE(d1.isSubType(typeid(Base).hash_code()));
    EXPECT_FALSE(d1.isSubType(typeid(OtherDerived).hash_code()));
}

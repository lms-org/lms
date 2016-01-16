#include "gtest/gtest.h"
#include "lms/endian.h"

TEST(Endian, uint16) {
    using lms::Endian;
    ASSERT_EQ(0xFECAu, Endian::letoh(Endian::htobe(uint16_t(0xCAFEu))));
    ASSERT_EQ(0xFECAu, Endian::betoh(Endian::htole(uint16_t(0xCAFEu))));
}

TEST(Endian, uint32) {
    using lms::Endian;
    ASSERT_EQ(0xEFBEADDEu, Endian::letoh(Endian::htobe(uint32_t(0xDEADBEEFu))));
    ASSERT_EQ(0xEFBEADDEu, Endian::betoh(Endian::htole(uint32_t(0xDEADBEEFu))));
}

TEST(Endian, uint64) {
    using lms::Endian;
    ASSERT_EQ(0xEFBEADDEFECAEDFEu, Endian::letoh(Endian::htobe(uint64_t(0xFEEDCAFEDEADBEEFu))));
    ASSERT_EQ(0xEFBEADDEFECAEDFEu, Endian::betoh(Endian::htole(uint64_t(0xFEEDCAFEDEADBEEFu))));
}

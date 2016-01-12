#include "gtest/gtest.h"
#include "lms/endian.h"

TEST(Endian, uint16) {
    using lms::Endian;
    ASSERT_EQ(uint16_t(0xCAFEu), Endian::betoh(Endian::htobe(uint16_t(0xCAFEu))));
    ASSERT_EQ(uint16_t(0xCAFEu), Endian::letoh(Endian::htole(uint16_t(0xCAFEu))));
}

TEST(Endian, uint32) {
    using lms::Endian;
    ASSERT_EQ(uint32_t(0xDEADBEEFu), Endian::betoh(Endian::htobe(uint32_t(0xDEADBEEFu))));
    ASSERT_EQ(uint32_t(0xDEADBEEFu), Endian::letoh(Endian::htole(uint32_t(0xDEADBEEFu))));
}

TEST(Endian, uint64) {
    using lms::Endian;
    ASSERT_EQ(uint64_t(0xFEEDCAFEDEADBEEFu), Endian::betoh(Endian::htobe(uint64_t(0xFEEDCAFEDEADBEEFu))));
    ASSERT_EQ(uint64_t(0xFEEDCAFEDEADBEEFu), Endian::letoh(Endian::htole(uint64_t(0xFEEDCAFEDEADBEEFu))));
}

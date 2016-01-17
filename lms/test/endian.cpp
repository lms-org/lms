#include "gtest/gtest.h"
#include "lms/endian.h"

TEST(Endian, uint16) {
    using lms::Endian;
    ASSERT_EQ(0xFECAu, Endian::letoh(Endian::htobe(uint16_t(0xCAFEu))));
    ASSERT_EQ(0xFECAu, Endian::betoh(Endian::htole(uint16_t(0xCAFEu))));
}

TEST(Endian, int16) {
    using lms::Endian;
    ASSERT_EQ(int16_t(0xFECA), Endian::letoh(Endian::htobe(int16_t(0xCAFE))));
    ASSERT_EQ(int16_t(0xFECA), Endian::betoh(Endian::htole(int16_t(0xCAFE))));
}

TEST(Endian, uint32) {
    using lms::Endian;
    ASSERT_EQ(0xEFBEADDEu, Endian::letoh(Endian::htobe(uint32_t(0xDEADBEEFu))));
    ASSERT_EQ(0xEFBEADDEu, Endian::betoh(Endian::htole(uint32_t(0xDEADBEEFu))));
}

TEST(Endian, int32) {
    using lms::Endian;
    ASSERT_EQ(int32_t(0xEFBEADDE), Endian::letoh(Endian::htobe(int32_t(0xDEADBEEF))));
    ASSERT_EQ(int32_t(0xEFBEADDE), Endian::betoh(Endian::htole(int32_t(0xDEADBEEF))));
}

TEST(Endian, uint64) {
    using lms::Endian;
    ASSERT_EQ(0xEFBEADDEFECAEDFEu, Endian::letoh(Endian::htobe(uint64_t(0xFEEDCAFEDEADBEEFu))));
    ASSERT_EQ(0xEFBEADDEFECAEDFEu, Endian::betoh(Endian::htole(uint64_t(0xFEEDCAFEDEADBEEFu))));
}

TEST(Endian, int64) {
    using lms::Endian;
    ASSERT_EQ(int64_t(0xEFBEADDEFECAEDFE), Endian::letoh(Endian::htobe(int64_t(0xFEEDCAFEDEADBEEF))));
    ASSERT_EQ(int64_t(0xEFBEADDEFECAEDFE), Endian::betoh(Endian::htole(int64_t(0xFEEDCAFEDEADBEEF))));
}

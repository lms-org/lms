#include "lms/endian.h"

#ifdef __APPLE__
#include <libkern/OSByteOrder.h>
#else
#include <endian.h>
#endif

namespace lms {

#ifdef __APPLE__

template<> uint16_t Endian::htobe<uint16_t>(uint16_t x) { return OSSwapHostToBigInt16(x); }
template<> uint32_t Endian::htobe<uint32_t>(uint32_t x) { return OSSwapHostToBigInt32(x); }
template<> uint64_t Endian::htobe<uint64_t>(uint64_t x) { return OSSwapHostToBigInt64(x); }

template<> uint16_t Endian::htole<uint16_t>(uint16_t x) { return OSSwapHostToLittleInt16(x); }
template<> uint32_t Endian::htole<uint32_t>(uint32_t x) { return OSSwapHostToLittleInt32(x); }
template<> uint64_t Endian::htole<uint64_t>(uint64_t x) { return OSSwapHostToLittleInt64(x); }

template<> uint16_t Endian::betoh<uint16_t>(uint16_t x) { return OSSwapBigToHostInt16(x); }
template<> uint32_t Endian::betoh<uint32_t>(uint32_t x) { return OSSwapBigToHostInt32(x); }
template<> uint64_t Endian::betoh<uint64_t>(uint64_t x) { return OSSwapBigToHostInt64(x); }

template<> uint16_t Endian::letoh<uint16_t>(uint16_t x) { return OSSwapLittleToHostInt16(x); }
template<> uint32_t Endian::letoh<uint32_t>(uint32_t x) { return OSSwapLittleToHostInt32(x); }
template<> uint64_t Endian::letoh<uint64_t>(uint64_t x) { return OSSwapLittleToHostInt64(x); }

#else

template<> uint16_t Endian::htobe<uint16_t>(uint16_t x) { return htobe16(x); }
template<> uint32_t Endian::htobe<uint32_t>(uint32_t x) { return htobe32(x); }
template<> uint64_t Endian::htobe<uint64_t>(uint64_t x) { return htobe64(x); }

template<> uint16_t Endian::htole<uint16_t>(uint16_t x) { return htole16(x); }
template<> uint32_t Endian::htole<uint32_t>(uint32_t x) { return htole32(x); }
template<> uint64_t Endian::htole<uint64_t>(uint64_t x) { return htole64(x); }

template<> uint16_t Endian::betoh<uint16_t>(uint16_t x) { return be16toh(x); }
template<> uint32_t Endian::betoh<uint32_t>(uint32_t x) { return be32toh(x); }
template<> uint64_t Endian::betoh<uint64_t>(uint64_t x) { return be64toh(x); }

template<> uint16_t Endian::letoh<uint16_t>(uint16_t x) { return le16toh(x); }
template<> uint32_t Endian::letoh<uint32_t>(uint32_t x) { return le32toh(x); }
template<> uint64_t Endian::letoh<uint64_t>(uint64_t x) { return le64toh(x); }

#endif

}  // namespace lms

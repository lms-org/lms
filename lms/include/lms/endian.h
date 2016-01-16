#ifndef LMS_ENDIAN_H
#define LMS_ENDIAN_H

#include <cstdint>

namespace lms {

/**
 * @brief Utility class to swap byte order from/to big or little endian.
 */
class Endian {
public:
    /**
     * @brief This class may not be instantiated.
     */
    Endian() = delete;

    /**
     * @brief Convert host order to big endian.
     */
    template<typename T>
    static inline T htobe(T x);

    /**
     * @brief Convert host order to little endian.
     */
    template<typename T>
    static inline T htole(T x);

    /**
     * @brief Convert big endian to host order.
     */
    template<typename T>
    static inline T betoh(T x);

    /**
     * @brief Convert little endian to host order.
     */
    template<typename T>
    static inline T letoh(T x);
};

template<> uint16_t Endian::htobe<uint16_t>(uint16_t x);
template<> uint32_t Endian::htobe<uint32_t>(uint32_t x);
template<> uint64_t Endian::htobe<uint64_t>(uint64_t x);

template<> uint16_t Endian::htole<uint16_t>(uint16_t x);
template<> uint32_t Endian::htole<uint32_t>(uint32_t x);
template<> uint64_t Endian::htole<uint64_t>(uint64_t x);

template<> uint16_t Endian::betoh<uint16_t>(uint16_t x);
template<> uint32_t Endian::betoh<uint32_t>(uint32_t x);
template<> uint64_t Endian::betoh<uint64_t>(uint64_t x);

template<> uint16_t Endian::letoh<uint16_t>(uint16_t x);
template<> uint32_t Endian::letoh<uint32_t>(uint32_t x);
template<> uint64_t Endian::letoh<uint64_t>(uint64_t x);

}  // namespace lms

#endif // LMS_ENDIAN_H

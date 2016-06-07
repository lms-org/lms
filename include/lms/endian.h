#ifndef LMS_ENDIAN_H
#define LMS_ENDIAN_H

#include <cstdint>
#include <type_traits>

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
     * @brief Convert unsigned integers from host order to big endian.
     */
    template <typename T>
    static inline typename std::enable_if<std::is_unsigned<T>::value, T>::type
    htobe(T x);

    /**
     * @brief Convert signed integers from host order to big endian.
     */
    template <typename T>
    static inline typename std::enable_if<std::is_signed<T>::value, T>::type
    htobe(T x) {
        return static_cast<T>(
            htobe(static_cast<typename std::make_unsigned<T>::type>(x)));
    }

    /**
     * @brief Convert unsigned integers from host order to little endian.
     */
    template <typename T>
    static inline typename std::enable_if<std::is_unsigned<T>::value, T>::type
    htole(T x);

    /**
     * @brief Convert signed integers from host order to little endian.
     */
    template <typename T>
    static inline typename std::enable_if<std::is_signed<T>::value, T>::type
    htole(T x) {
        return static_cast<T>(
            htole(static_cast<typename std::make_unsigned<T>::type>(x)));
    }

    /**
     * @brief Convert unsigned integers from big endian to host order.
     */
    template <typename T>
    static inline typename std::enable_if<std::is_unsigned<T>::value, T>::type
    betoh(T x);

    /**
     * @brief Convert signed integers from big endian to host order.
     */
    template <typename T>
    static inline typename std::enable_if<std::is_signed<T>::value, T>::type
    betoh(T x) {
        return static_cast<T>(
            betoh(static_cast<typename std::make_unsigned<T>::type>(x)));
    }

    /**
     * @brief Convert unsigned integers from little endian to host order.
     */
    template <typename T>
    static inline typename std::enable_if<std::is_unsigned<T>::value, T>::type
    letoh(T x);

    /**
     * @brief Convert signed integers from little endian to host order.
     */
    template <typename T>
    static inline typename std::enable_if<std::is_signed<T>::value, T>::type
    letoh(T x) {
        return static_cast<T>(
            letoh(static_cast<typename std::make_unsigned<T>::type>(x)));
    }
};

template <> uint16_t Endian::htobe<uint16_t>(uint16_t x);
template <> uint32_t Endian::htobe<uint32_t>(uint32_t x);
template <> uint64_t Endian::htobe<uint64_t>(uint64_t x);

template <> uint16_t Endian::htole<uint16_t>(uint16_t x);
template <> uint32_t Endian::htole<uint32_t>(uint32_t x);
template <> uint64_t Endian::htole<uint64_t>(uint64_t x);

template <> uint16_t Endian::betoh<uint16_t>(uint16_t x);
template <> uint32_t Endian::betoh<uint32_t>(uint32_t x);
template <> uint64_t Endian::betoh<uint64_t>(uint64_t x);

template <> uint16_t Endian::letoh<uint16_t>(uint16_t x);
template <> uint32_t Endian::letoh<uint32_t>(uint32_t x);
template <> uint64_t Endian::letoh<uint64_t>(uint64_t x);

} // namespace lms

#endif // LMS_ENDIAN_H

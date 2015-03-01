#ifndef LMS_TYPE_STATIC_IMAGE_H
#define LMS_TYPE_STATIC_IMAGE_H

#include <algorithm>

namespace lms { namespace type {

/**
 * @brief StaticImage serves as a data channel type, as an replacement
 * for char* with size parameter.
 *
 * A static image is a two dimensional array where the size
 * and type already given in compile time as type parameters.
 *
 * The data is stored row-by-row in a simple one-dimensional array.
 * If you want to be cache-optimized you should read the array row by row.
 *
 * @author Hans Kirchner
 */
template<int W, int H, typename T>
class StaticImage {
public:
    /**
     * @brief Width of the image
     */
    static constexpr int width = W;

    /**
     * @brief Height of the image
     */
    static constexpr int height = H;

    /**
     * @brief Width multiplied by height
     */
    static constexpr int size = W * H;

    /**
     * @brief Reference to a value in the image.
     *
     * Usage: image(1,2)
     * Result: data at column #1 and row #2
     *
     * @param x column
     * @param y row
     * @return value
     */
    T& operator() (int x, int y) {
        return m_data[x + width * y];
    }

    /**
     * @brief Const reference to a value in the image.
     * @param x column
     * @param y row
     * @return value
     */
    const T& operator() (int x, int y) const {
        return m_data[x + width * y];
    }

    /**
     * @brief Fill the whole image with the given value.
     *
     * NOTE: This calls std::fill_n. For simple types like
     * char this is usually as fast as a memset.
     *
     * @param value the value
     */
    void fill(const T& value) {
        std::fill_n(m_data, size, value);
    }

    /**
     * @brief Swap the content with the another static image.
     *
     * NOTE: You can only swap with the same type of static images.
     *
     * @param image another image to swap content with
     */
    void swap(StaticImage<W,H,T> &image) {
        std::swap(m_data, image.m_data);
    }

private:
    T m_data[size];
};

}}

#endif /* LMS_TYPE_STATIC_IMAGE_H */

#ifndef LMS_TYPE_DYNAMIC_IMAGE_H
#define LMS_TYPE_DYNAMIC_IMAGE_H

#include <memory>
#include <algorithm>
#include <utility>
#include <cstdint>

namespace lms {
namespace type {

/**
 * @brief DynamicImage serves as a data channel type, as an replacement
 * for char* with size parameter.
 *
 * A dynamic image is a two dimensional array where the type is already
 * given during compile time. The width and height can be changed during runtime.
 *
 * The data is stored row-by-row in a simple one-dimensional array.
 * If you want to be cache-optimized you should read the array row by row.
 *
 * @author Hans Kirchner
 */
class DynamicImage {
public:
    /**
     * @brief Create an image of size zero.
     */
    constexpr DynamicImage() : m_width(0), m_height(0), m_bpp(0), m_size(0) {
    }

    /**
     * @brief Create a new dynamic image with the given width and height.
     *
     * The type is always given during compile time as the type parameter.
     *
     * @param width width of the image (x-direction)
     * @param height height of the image (y-direction)
     */
    DynamicImage(int width, int height, int bpp) : m_width(width), m_height(height),
        m_bpp(bpp), m_size(width * height * bpp), m_data(new std::uint8_t[m_size]) {
    }

    /**
     * @brief Copy constructor
     * @param DynamicImage object to copy
     */
    DynamicImage(const DynamicImage &obj) : m_width(obj.m_width), m_height(obj.m_height),
        m_bpp(obj.m_bpp), m_size(obj.m_size), m_data(new std::uint8_t[m_size]) {
        std::copy(obj.m_data.get(), obj.m_data.get() + m_size, m_data.get());
    }

    /**
     * @brief Move constructor
     * @param object to move
     */
    DynamicImage(DynamicImage &&obj) = default;

    /**
     * @brief Copy assignment operator
     * @param rhs right side of the assignment
     * @return this
     */
    DynamicImage& operator=(const DynamicImage &rhs) {
        this->m_width = rhs.m_width;
        this->m_height = rhs.m_height;
        this->m_bpp = rhs.m_bpp;
        this->m_size = rhs.m_size;
        m_data.reset(new std::uint8_t[m_size]);
        std::copy(rhs.m_data.get(), rhs.m_data.get() + m_size, m_data.get());
        return *this;
    }

    /**
     * @brief Move assignment operator
     * @param rhs right side of the assignment
     * @return this
     */
    DynamicImage& operator=(DynamicImage &&rhs) = default;

    /**
     * @brief Return a reference to the value at the given index.
     *
     * The image is stored row-by-row in an array.
     *
     * The index must be in the range [0, size).
     *
     * @param index index to look up
     * @return reference to value at the given index
     */
    std::uint8_t& operator[] (int index) {
        return m_data[index];
    }


    /**
     * @brief Return a const reference to the value at the given index.
     *
     * The image is stored row-by-row in an array.
     *
     * The index must be in the range [0, size).
     *
     * @param index index to look up
     * @return const reference to value at the given index
     */
    const std::uint8_t& operator[] (int index) const {
        return m_data[index];
    }

    /**
     * @brief Resize the image to the new width and height.
     *
     * NOTE: This will discard all data that was in the image before.
     *
     * @param width new width of the image
     * @param height new height of the image
     */
    void resize(int width, int height, int bpp) {
        *this = DynamicImage(width, height, bpp);
    }

    /**
     * @brief Fill the dynamic image with the given value.
     * @param value
     */
    void fill(std::uint8_t value) {
        std::fill_n(m_data.get(), m_size, value);
    }

    /**
     * @brief Return the width of the image
     * @return width
     */
    int width() const {
        return m_width;
    }

    /**
     * @brief Return the height of the image
     * @return height
     */
    int height() const {
        return m_height;
    }

    /**
     * @brief Return the bits per pixel setting.
     * @return bits per pixel
     */
    int bpp() const {
        return m_bpp;
    }

    /**
     * @brief Return the size of the image, that is width * height.
     * @return size of the image
     */
    int size() const {
        return m_size;
    }

    /**
     * @brief Return a raw data pointer
     *
     * NOTE: Do NOT delete this pointer.
     * The image object will delete it automatically.
     *
     * @return data pointer
     */
    std::uint8_t* data() {
        return m_data.get();
    }

    /**
     * @brief Return a const raw data pointer
     *
     * NOTE: Do NOT delete this pointer
     * The image object will delete it automatically.
     *
     * @return data pointer
     */
    std::uint8_t* data() const {
        return m_data.get();
    }

private:
    int m_width;
    int m_height;
    int m_bpp;
    int m_size;
    std::unique_ptr<std::uint8_t[]> m_data;
};

} // namespace type
} // namespace lms

#endif /* LMS_TYPE_DYNAMIC_IMAGE_H */

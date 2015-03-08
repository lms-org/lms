#ifndef LMS_TYPE_DYNAMIC_IMAGE_H
#define LMS_TYPE_DYNAMIC_IMAGE_H

#include <memory>
#include <algorithm>
#include <utility>

namespace lms {
namespace type {

template<typename T>
class DynamicImage {
public:
    /**
     * @brief Create an image of size zero.
     */
    constexpr DynamicImage() : m_width(0), m_height(0), m_size(0) {
    }

    /**
     * @brief Create a new dynamic image with the given width and height.
     *
     * The type is always given during compile time as the type parameter.
     *
     * @param width width of the image (x-direction)
     * @param height height of the image (y-direction)
     */
    DynamicImage(int width, int height) : m_width(width), m_height(height),
        m_size(width * height), m_data(new T[m_size]) {
    }

    /**
     * @brief Copy constructor
     * @param DynamicImage object to copy
     */
    DynamicImage(const DynamicImage<T> &obj) : m_width(obj.m_width), m_height(obj.m_height),
        m_size(obj.m_size), m_data(new T[m_size]) {
        std::copy(obj.m_data.get(), obj.m_data.get() + m_size, m_data.get());
    }

    /**
     * @brief Move constructor
     * @param object to move
     */
    constexpr DynamicImage(DynamicImage<T> &&obj) : m_width(obj.m_width), m_height(obj.m_height),
        m_size(obj.m_size), m_data(std::move(obj.m_data)) {
    }

    /**
     * @brief Copy assignment operator
     * @param rhs right side of the assignment
     * @return this
     */
    DynamicImage& operator=(const DynamicImage<T> &rhs) {
        this->m_width = rhs.m_width;
        this->m_height = rhs.m_height;
        this->m_size = rhs.m_size;
        m_data.reset(new T[m_size]);
        std::copy(rhs.m_data.get(), rhs.m_data.get() + m_size, m_data.get());
        return *this;
    }

    /**
     * @brief Move assignment operator
     * @param rhs right side of the assignment
     * @return this
     */
    DynamicImage& operator=(DynamicImage<T> &&rhs) {
        this->m_width = rhs.m_width;
        this->m_height = rhs.m_height;
        this->m_size = rhs.m_size;
        this->m_data = std::move(rhs.m_data);
        return *this;
    }

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
    T& operator[] (int index) {
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
    const T& operator[] (int index) const {
        return m_data[index];
    }

    /**
     * @brief Fill the dynamic image with the given value.
     * @param value
     */
    void fill(const T& value) {
        std::fill_n(m_data, m_size, value);
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
    T* data() {
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
    T* data() const {
        return m_data.get();
    }

private:
    int m_width;
    int m_height;
    int m_size;
    std::unique_ptr<T[]> m_data;
};

} // namespace type
} // namespace lms

#endif /* LMS_TYPE_DYNAMIC_IMAGE_H */

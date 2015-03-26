#ifndef LMS_SERIALIZABLE_H
#define LMS_SERIALIZABLE_H

#include <iostream>

namespace lms {

/**
 * @brief Abstract super class for all data channels
 * that should be serializable.
 *
 * Provides methods to serialize and deserialize the
 * data channel.
 */
class Serializable {
    /**
     * @brief Serialize the object into the given output stream.
     * @param os output stream to write in
     */
    virtual void serialize(std::ostream &os) = 0;

    /**
     * @brief Deserialize the object from the given input stream.
     * @param is input stream to read from
     */
    virtual void deserialize(std::istream &is) = 0;
};

}  // namespace lms

#endif /* LMS_SERIALIZABLE_H */

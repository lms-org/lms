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
public:
    virtual ~Serializable() {}

    /**
     * @brief Serialize the object into the given output stream.
     * @param os output stream to write in
     */
    virtual void lmsSerialize(std::ostream &os) const = 0;

    /**
     * @brief Deserialize the object from the given input stream.
     * @param is input stream to read from
     */
    virtual void lmsDeserialize(std::istream &is) = 0;
};

} // namespace lms

#endif /* LMS_SERIALIZABLE_H */

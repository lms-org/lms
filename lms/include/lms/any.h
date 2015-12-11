#ifndef LMS_ANY_H
#define LMS_ANY_H

namespace lms {

/**
 * @brief The Any struct used to access dataChannels for which you don't know the type.
 *
 * This is class is a virtual super class for all data channel types.
 * Nevertheless you should not derive from this class.
 */
struct Any {
    void operator* () {
        throw std::runtime_error("Any is afraid of stars");
    }

    void* operator-> () {
        throw new std::runtime_error("Robin Hood won't give you his arrow");
    }
};

}  // namespace lms

#endif // LMS_ANY_H

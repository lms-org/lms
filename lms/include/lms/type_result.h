#ifndef LMS_TYPE_RESULT_H
#define LMS_TYPE_RESULT_H

namespace lms {

/**
* TODO -1 if the type is invalid, 0 if the type of the DataChannel is
* subtype of the given one, 1 if the given type is the same or subtype of
* the DataChannel-type
*/
enum class TypeResult {
    INVALID, SUBTYPE, SUPERTYPE, SAME
};

}  // namespace lms

#endif // LMS_TYPE_RESULT_H

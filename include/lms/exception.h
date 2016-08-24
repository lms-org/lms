#ifndef LMS_EXCEPTION_H
#define LMS_EXCEPTION_H

#include <exception>
#include <string>

#define LMS_EXCEPTION(MESSAGE)                                                 \
    throw lms::LmsException(MESSAGE, __FILE__, __LINE__)

namespace lms {

/**
 * @brief Convenient wrapper class around std::exception.
 */
class LmsException : public std::exception {
private:
    std::string message;

public:
    /**
     * @brief Create LMSException with a custom message.
     */
    LmsException(const std::string &message) : message(message) {}

    /**
     * @brief Create LMSException with a custom message with file and line
     *information.
     *
     * This constructor should not be called directly, use LMS_EXCEPTION
     *instead.
     */
    LmsException(const std::string &message, const std::string &file, int line)
        : message(file + ":" + std::to_string(line) + " " + message) {}

    virtual const char *what() const noexcept { return message.c_str(); }

    virtual ~LmsException() {}
};

class XmlParserException : public LmsException {
public:
    XmlParserException(const std::string &message) : LmsException(message) {}
};

} // namespace lms

#endif // LMS_EXCEPTION_H

#ifndef LMS_LOGGING_FILE_SINK_H
#define LMS_LOGGING_FILE_SINK_H

#include <fstream>

#include "lms/logging/console_sink.h"

namespace lms {
namespace logging {

/**
 * @brief FileSink is a logging sink implementation that logs into files.
 *
 * The current implementation extends the console sink.
 */
class FileSink : public ConsoleSink {
public:
    /**
     * @brief Create a file sink that is writing into the given file.
     * @param logFile absolute path to a logging file
     */
    FileSink(const std::string &logFile);
private:
    std::ofstream m_ofs;
};

}  // namespace logging
}  // namespace lms

#endif /* LMS_LOGGING_FILE_SINK_H */

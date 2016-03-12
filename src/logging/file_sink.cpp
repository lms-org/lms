#include "lms/logging/file_sink.h"

namespace lms {
namespace logging {

FileSink::FileSink(const std::string &logFile) : ConsoleSink(m_ofs, false, true) {
    m_ofs.open(logFile);
}

}  // namespace logging
}  // namespace lms

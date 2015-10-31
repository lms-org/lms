#ifndef LMS_PROFILER_H
#define LMS_PROFILER_H

#include <string>
#include <mutex>
#include <fstream>

#include <lms/extra/time.h>

namespace lms {

/**
 * @brief Collect profiling data and write it to a file.
 *
 * After creating and enabling a profiler instance you are usually doing this:
 *
 * ~~~
 * profiler.markBegin("MyProcess");
 * doProcess(); // execute process
 * profiler.markEnd("MyProcess");
 * ~~~
 */
class Profiler {
public:
    /**
     * @brief Create a disabled profiler.
     *
     * markBegin/markEnd are a no-op in diabled mode.
     * Enable the profiler with the enable() method.
     */
    Profiler();

    /**
     * @brief Close the internal file descriptor if previously enabled.
     */
    ~Profiler();

    /**
     * @brief Enables this profiler and write marks to the given file.
     * @param file path to a file
     */
    void enable(const std::string &file);

    /**
     * @brief Mark the begin of a process identified by a label.
     * @param label the same label should be used in markEnd()
     */
    void markBegin(const std::string &label);

    /**
     * @brief Mark the end of a process identified by a label.
     * @param label the same label that was used in markBegin()
     */
    void markEnd(const std::string &label);
private:
    enum Type {
        BEGIN = 0, END
    };

    void mark(Type type, const std::string& label);

    bool m_enabled;
    lms::extra::PrecisionTime m_lastTimestamp;
    std::mutex m_mutex;
    std::ofstream m_stream;
};

}  // namespace lms

#endif /* LMS_PROFILER_H */

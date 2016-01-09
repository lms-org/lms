#ifndef LMS_PROFILER_H
#define LMS_PROFILER_H

#include <string>
#include <mutex>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <memory>

#include "lms/time.h"

namespace lms {
namespace internal {

class DebugServer;

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
     * @brief Mark the begin of a process identified by a label.
     * @param label the same label should be used in markEnd()
     */
    void markBegin(const std::string &label);

    /**
     * @brief Mark the end of a process identified by a label.
     * @param label the same label that was used in markBegin()
     */
    void markEnd(const std::string &label);

    enum Type : std::uint8_t {
        BEGIN = 0, END, MAPPING
    };

    class ProfilingListener {
    public:
        virtual ~ProfilingListener() {}
        virtual void onMarker(Type type, lms::Time now, std::string const& label) =0;
    };

    void appendListener(ProfilingListener *listener);
private:
    void mark(Type type, const std::string& label);

    std::vector<std::unique_ptr<ProfilingListener>> m_listeners;
};

class FileProfiler : public Profiler::ProfilingListener {
public:
    FileProfiler(std::string const& file);
    ~FileProfiler();

    void onMarker(Profiler::Type type, lms::Time now, std::string const& label) override;
private:
    std::ofstream m_stream;

    std::mutex m_mutex;
    lms::Time m_lastTimestamp;

    typedef std::unordered_map<std::string, size_t> MappingType;
    MappingType m_stringMapping;
};

class DebugServerProfiler : public Profiler::ProfilingListener {
public:
    DebugServerProfiler(DebugServer *server);

    void onMarker(Profiler::Type type, lms::Time now, std::string const& label) override;
private:
    DebugServer * m_server;
};

}  // namespace internal
}  // namespace lms

#endif /* LMS_PROFILER_H */

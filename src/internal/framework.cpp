#include "./framework.h"
#include <sys/stat.h>
#include <string>
#include <memory>
#include <csignal>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include "backtrace_formatter.h"
#include "lms/protobuf_socket.h"
#include "os.h"

namespace lms {
namespace internal {

Framework::Framework(const std::string &mainConfigFilePath)
    : m_executionManager(*this), logger("lms.Framework"),
      mainConfigFilePath(mainConfigFilePath), m_running(false), is_debug(false) {

    // start();
}

void Framework::addSearchPath(const std::string &path) {
    m_loader.addSearchPath(path);
}

void Framework::start() {
    bool firstRun = true;
    m_running = true;

    while (m_running) {
        // config monitor stuff
        if (firstRun || configMonitor.hasChangedFiles()) {
            logger.info() << "Reload configs";
            configMonitor.unwatchAll();
            RuntimeInfo runtime;
            for(const auto &f: flags) {
                logger.info("flag") << f;
            }
            XmlParser parser(runtime, flags);
            parser.parseFile(mainConfigFilePath);

            for (auto error : parser.errors()) {
                logger.error("XML") << error;
            }

            if(parser.errors().size() > 0) {
                logger.error("XML") << "Fix XML errors first. Shutting down...";
                m_running = false;
                break;
            }

            try {
                logger.time("updateSystem");
                if(! updateSystem(runtime)) {
                    m_running = false;
                    break;
                }
                logger.timeEnd("updateSystem");
            } catch(std::exception const &ex) {
                logger.error() << lms::typeName(ex) << ": " <<  ex.what();
                m_running = false;
                break;
            }

            if(isDebug()) {
                printOverview();
                printModuleChannelGraph();
            }

            for (auto file : parser.files()) {
                configMonitor.watch(file);
            }
        }

        if(!cycle()) {
            m_running = false;
        }
        firstRun = false;
    }

    shutdown();

    logger.info() << "Stopped";
}

bool Framework::updateSystem(const RuntimeInfo &info) {
    if(isDebug()) {
        logger.debug() << "updateSystem()";
    }

    // Update clock
    m_clock.cycleTime(info.clock.cycle);
    m_clock.enabledSleep(info.clock.sleep);
    m_clock.enabledSlowWarning(info.clock.slowWarnings);
    m_clock.enabledCompensate(info.clock.sleepCompensate);

    // Update or load services
    for (const ServiceInfo &serviceInfo : info.services) {
        if(isDebug()) {
            logger.debug() << "Loading service " << serviceInfo.name;
        }
        auto it = services.find(serviceInfo.name);
        if (it == services.end()) {
            // Not loaded yet
            std::shared_ptr<Service> service(m_loader.loadService(serviceInfo));

            service->initBase(serviceInfo);

            try {
                if (!service->init()) {
                    logger.error() << "Service " << serviceInfo.name
                                   << " failed to init()";
                    return false;
                }
            } catch (std::exception const &ex) {
                logger.error() << serviceInfo.name << " throws "
                               << lms::typeName(ex) << " : " << ex.what();
                return false;
            }

            services[serviceInfo.name] = service;
        } else {
            // already loaded
            it->second->initBase(serviceInfo);
            it->second->configsChanged();
        }
    }

    // Update or load modules
    for (const ModuleInfo &moduleInfo : info.modules) {
        if(isDebug()) {
            logger.debug() << "Loading module " << moduleInfo.name;
        }
        auto it = modules.find(moduleInfo.name);
        if (it == modules.end()) {
            // Not yet loaded
            std::shared_ptr<Module> module(m_loader.loadModule(moduleInfo));
            module->initBase(moduleInfo, this);

            try {
                if (!module->init()) {
                    logger.error() << "Module " << moduleInfo.name
                                   << " failed to init()";
                    return false;
                }
            } catch (std::exception const &ex) {
                logger.error() << moduleInfo.name << " throws "
                               << lms::typeName(ex) << " : " << ex.what();
                return false;
            }

            modules[moduleInfo.name] = module;
        } else {
            it->second->initBase(moduleInfo, this);
            it->second->configsChanged();
        }
    }

    if(isDebug()) {
        logger.debug() << "updated system";
    }

    return true;
}

void Framework::shutdown() {
    // Shutdown services
    for (auto &service : services) {
        try {
            service.second->destroy();
        } catch (std::exception const &ex) {
            logger.error() << service.first << " throws " << lms::typeName(ex)
                           << " : " << ex.what();
        }
    }

    // Shutdown modules
    for (auto &module : modules) {
        try {
            module.second->destroy();
        } catch (std::exception const &ex) {
            logger.error() << module.first << " throws " << lms::typeName(ex)
                           << " : " << ex.what();
        }
    }

    services.clear();
    modules.clear();
}

Framework::~Framework() {
    shutdown();
}

bool Framework::cycle() {
    if(modules.size() == 0) {
        logger.error() << "No modules enabled. Check your config file. Shutting down ...";
        return false;
    }

    m_clock.beforeLoopIteration();
    executionManager().validate(modules);

    {
        std::lock_guard<std::mutex> lock(m_recordingMutex);
        if(m_recordingState == RecordingState::LOAD) {
            for(auto &stream : m_recordingStreams) {
                m_dataManager.writeChannel<lms::Any>(stream.first).deserialize(stream.second);
            }
        }
    }

    logger.time("cycle");
    m_executionManager.loop();
    logger.timeEnd("cycle");

    {
        std::lock_guard<std::mutex> lock(m_recordingMutex);
        if(m_recordingState == RecordingState::SAVE) {
            for(auto &stream : m_recordingStreams) {
                m_dataManager.readChannel<lms::Any>(stream.first).serialize(stream.second);
            }
        }
    }

    return true;
}

std::shared_ptr<Service> Framework::getService(std::string const &name) {
    return services[name];
}

bool Framework::isDebug() const {
    return is_debug; /* TODO make this configurable */
}

void Framework::setDebug(bool debug) {
    this->is_debug = debug;
}

DataManager &Framework::dataManager() { return m_dataManager; }

ExecutionManager &Framework::executionManager() { return m_executionManager; }

std::string Framework::loadPath() const { return m_loadLogPath; }

std::string Framework::loadLogObject(std::string const &name, bool isDir) {
    if (!isEnableLoad()) {
        throw std::runtime_error(
            "Command line option --enable-load was not specified");
    }

    std::string logobj = m_loadLogPath + "/" + name;

    return isDir ? logobj + "/" : logobj;
}

std::string Framework::saveLogObject(std::string const &name, bool isDir) {
    if (!isEnableSave()) {
        throw std::runtime_error(
            "Command line option --enable-save was not specified");
    }

    std::string logobj = m_saveLogPath + "/" + name;

    if (isDir) {
        mkdir(logobj.c_str(), MODE);
    }

    return isDir ? logobj + "/" : logobj;
}

void Framework::enableLoad(const std::string &path) {
    m_isEnableLoad = true;
    m_loadLogPath = path;
}

void Framework::enableSave(const std::string &path) {
    m_isEnableSave = true;
    m_saveLogPath = path;
}

bool Framework::isEnableLoad() const { return m_isEnableLoad; }

bool Framework::isEnableSave() const { return m_isEnableSave; }

void Framework::addFlag(const std::string &flag) {
    flags.push_back(flag);
}

void Framework::signal(int signal) {
    switch(signal) {
    case SIGSEGV:
        {
        std::ofstream of(std::string("/tmp/lms-segfault-") +
                         std::to_string(getpid()) + "-" + std::to_string(std::time(NULL)) + ".txt");
        printStacktrace(of);
        of.close();
        _exit(1);
        }
        break;
    case SIGINT:
        m_running = false;
        break;
    }
}

void Framework::stop() {
    m_running = false;
}

void Framework::printOverview() {
    logger.info("overview") << "Modules (" << modules.size() << ")";
    for(const auto &module : modules) {
        logger.info("overview") << "  " << module.first;
    }
    logger.info("overview") << "Services (" << services.size() << ")";
    for(const auto &service : services) {
        logger.info("overview") << "  " << service.first;
    }
}

void Framework::printModuleChannelGraph() {
    const auto &graph = m_executionManager.getModuleChannelGraph();
    for(const auto &node : graph) {
        logger.info("graph") << node.first;
        for(const auto &access : node.second) {
            logger.info("graph") << "  "
                << access.module->getName()
                << " " << static_cast<int>(access.permission)
                << " " << access.priority;
        }
    }
}

void Framework::printDAG() {
    const auto &dag = m_executionManager.getDAG();
    for(const auto &node : dag) {
        std::string dependencies;
        for(const auto &dependency : node.second) {
            dependencies += dependency->getName() + " ";
        }
        logger.info("dag") << node.first->getName() << " ( " << dependencies << ")";
    }
}

void Framework::startCommunicationThread(int sock) {
    m_communicationThread = std::thread([sock, this] () {
        ProtobufSocket socket(sock);
        lms::Request message;
        while(socket.readMessage(message) == ProtobufSocket::OK) {
            using C = lms::Request::Runtime::ContentCase;
            if(!message.has_runtime()) {
                logger.error() << "Received unknown message via commSocket";
                continue;
            }

            switch(message.runtime().content_case()) {
            case C::kProfiling:
                {
                std::map<std::string, logging::Trace<double>> measurements;
                logging::Context::getDefault().profilingSummary(measurements);
                if(message.runtime().profiling().reset()) {
                    logging::Context::getDefault().resetProfiling();
                }
                lms::Response summary;
                summary.mutable_profiling_summary(); // set type of response even if no measurements available
                for(const auto &pair : measurements) {
                    Response::ProfilingSummary::Trace *trace = summary.mutable_profiling_summary()->add_traces();
                    trace->set_name(pair.first);
                    trace->set_count(pair.second.count());
                    trace->set_avg(pair.second.avg());
                    if(pair.second.count() > 0) {
                        trace->set_min(pair.second.min());
                        trace->set_max(pair.second.max());
                    } else {
                        trace->set_min(0);
                        trace->set_max(0);
                    }
                    trace->set_std(pair.second.std());
                    if(pair.second.hasBegin()) {
                        trace->set_running_since((lms::Time::now() - pair.second.begin()).micros());
                    }
                }
                socket.writeMessage(summary);
                }
                break;
            case C::kFilter:
                // TODO
                {
                logging::Context::getDefault().setLevel(
                    static_cast<logging::Level>(message.runtime().filter().log_level()));
                }
                break;
            case C::kStartRecording:
                {
                // TODO better do this during a cycle
                std::lock_guard<std::mutex> lock(m_recordingMutex);
                if(m_recordingState == RecordingState::NONE) {
                    m_recordingState = RecordingState::SAVE;
                    const auto &channels = message.runtime().start_recording().channels();
                    std::string path = homepath() + "/lmslogs/temp";
                    ::mkdir(path.c_str(), MODE);
                    for(int i = 0; i < channels.size(); i++) {
                        std::fstream &stream = m_recordingStreams[channels.Get(i)];
                        stream.open(path + "/" + channels.Get(i), std::fstream::out);
                    }
                } else {
                    logger.error() << "Can't start recording.";
                }
                }
                break;
            case C::kStopRecording:
                {
                // TODO better do this during a cycle
                std::lock_guard<std::mutex> lock(m_recordingMutex);
                if(m_recordingState == RecordingState::SAVE) {
                    for(auto &stream : m_recordingStreams) {
                        stream.second.close();
                    }
                    std::string oldPath = homepath() + "/lmslogs/temp";
                    std::string newPath = homepath() + "/lmslogs/" + message.runtime().stop_recording().tag();
                    ::rename(oldPath.c_str(), newPath.c_str());
                    m_recordingState = NONE;
                } else {
                    logger.error() << "Can't stop recording";
                }
                }
                break;
            }


            /*std::lock_guard<std::mutex> lock(m_queueMutex);
            m_queue.push_back(message);*/


        }
    });
}

void Framework::loadRecordings(const std::string &absPath, const std::vector<std::string> &channels) {
    std::lock_guard<std::mutex> lock(m_recordingMutex);
    logger.info() << "Loading records from " << absPath;
    m_recordingState = RecordingState::LOAD;
    for(const auto &ch : channels) {
        std::string path = absPath + "/" + ch;
        logger.info() << "Loading channel " << ch << " from " + path;
        std::fstream &stream = m_recordingStreams[ch];
        stream.open(path, std::fstream::out);
    }
}

}  // namespace internal
}  // namespace lms

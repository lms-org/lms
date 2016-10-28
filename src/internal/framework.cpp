#include "./framework.h"
#include <sys/stat.h>
#include <string>
#include <memory>
#include <csignal>
#include <sys/types.h>
#include <unistd.h>
#include "backtrace_formatter.h"

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

            try {
                logger.time("updateSystem");
                if(! updateSystem(runtime)) {
                    m_running = false;
                }
                logger.timeEnd("updateSystem");
            } catch(std::exception const &ex) {
                logger.error() << lms::typeName(ex) << ": " <<  ex.what();
                m_running = false;
            }

            if(isDebug()) {
                printOverview();
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

    logger.info() << "Stopped";
}

bool Framework::updateSystem(const RuntimeInfo &info) {
    if(isDebug()) {
        logger.debug() << "updateSystem()";
    }

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

Framework::~Framework() {
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
}

bool Framework::cycle() {
    if(modules.size() == 0) {
        logger.error() << "No modules enabled. Check your config file. Shutting down ...";
        return false;
    }

    m_clock.beforeLoopIteration();
    executionManager().validate(modules);
    m_executionManager.loop();
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

bool Framework::isEnableLoad() const { return false; /* TODO */ }

bool Framework::isEnableSave() const { return false; /* TODO */ }

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

}  // namespace internal
}  // namespace lms

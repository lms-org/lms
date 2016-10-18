#include "./framework.h"
#include <sys/stat.h>
#include <string>
#include <memory>

namespace lms {
namespace internal {

Framework::Framework(const std::string &mainConfigFilePath)
    : m_executionManager(m_profiler, *this), logger("lms.Framework"),
      mainConfigFilePath(mainConfigFilePath), m_running(false) {

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
            XmlParser parser(runtime);
            parser.parseFile(mainConfigFilePath);

            updateSystem(runtime);

            for (auto error : parser.errors()) {
                logger.error() << error;
            }

            for (auto file : parser.files()) {
                configMonitor.watch(file);
            }
        }

        cycle();
        firstRun = false;
    }
}

void Framework::updateSystem(const RuntimeInfo &info) {
    // Update or load services
    for (const ServiceInfo &serviceInfo : info.services) {
        auto it = services.find(serviceInfo.name);
        if (it == services.end()) {
            // Not loaded yet
            std::shared_ptr<Service> service(m_loader.loadService(serviceInfo));

            service->initBase(serviceInfo);

            try {
                if (!service->init()) {
                    logger.error() << "Service " << serviceInfo.name
                                   << " failed to init()";
                    return;
                }
            } catch (std::exception const &ex) {
                logger.error() << serviceInfo.name << " throws "
                               << lms::typeName(ex) << " : " << ex.what();
                return;
            }

            it->second = service;
        } else {
            // already loaded
            it->second->initBase(serviceInfo);
            it->second->configsChanged();
        }
    }

    // Update or load modules
    for (const ModuleInfo &moduleInfo : info.modules) {
        auto it = modules.find(moduleInfo.name);
        if (it == modules.end()) {
            // Not yet loaded
            std::shared_ptr<Module> module(m_loader.loadModule(moduleInfo));
            module->initBase(moduleInfo, this);

            try {
                if (!module->init()) {
                    logger.error() << "Module " << moduleInfo.name
                                   << " failed to init()";
                    return;
                }
            } catch (std::exception const &ex) {
                logger.error() << moduleInfo.name << " throws "
                               << lms::typeName(ex) << " : " << ex.what();
                return;
            }

            it->second = module;
        } else {
            it->second->initBase(moduleInfo, this);
            it->second->configsChanged();
        }
    }
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
    m_clock.beforeLoopIteration();
    executionManager().validate(modules);
    m_executionManager.loop();
    return true;
}

Profiler &Framework::profiler() { return m_profiler; }

std::shared_ptr<Service> Framework::getService(std::string const &name) {
    return services[name];
}

bool Framework::isDebug() const {
    return true; /* TODO make this configurable */
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

}  // namespace internal
}  // namespace lms

#include <lms/framework.h>
#include <lms/executionmanager.h>
#include <pugixml.hpp>
#include <fstream>
#include <csignal>
#include <map>
#include <thread>
#include <cstdlib>
#include "lms/extra/backtrace_formatter.h"
#include "lms/logging/log_level.h"
#include "lms/extra/time.h"
#include "lms/type/module_config.h"
#include "lms/extra/string.h"

namespace lms{

std::string Framework::externalDirectory = EXTERNAL_DIR;
std::string Framework::configsDirectory = CONFIGS_DIR;

Framework::Framework(const ArgumentHandler &arguments) :
    logger("FRAMEWORK", &rootLogger), argumentHandler(arguments), executionManager(rootLogger),
    clockEnabled(false), clock(rootLogger), monitor(), monitorEnabled(false) {

    rootLogger.filter(std::unique_ptr<logging::LoggingFilter>(new logging::PrefixAndLevelFilter(
        arguments.argLoggingMinLevel(), arguments.argLoggingPrefixes())));

    SignalHandler::getInstance()
            .addListener(SIGINT, this)
            .addListener(SIGSEGV, this);

    logger.info() << "RunLevel " <<  arguments.argRunLevel();

    //parse framework config
    if(arguments.argRunLevel() >= RunLevel::CONFIG) {
        parseConfig(LoadConfigFlag::LOAD_EVERYTHING);
    }

    if(arguments.argRunLevel() >= RunLevel::ENABLE) {
        // enable modules after they were made available
        logger.info() << "Start enabling modules";
        for(ModuleToLoad mod : tempModulesToLoadList) {
            executionManager.enableModule(mod.name, mod.logLevel);
        }

        if(arguments.argRunLevel() == RunLevel::ENABLE) {
            executionManager.getDataManager().printMapping();
            executionManager.validate();
            executionManager.printCycleList();
        }
    }

    if(arguments.argRunLevel() >= RunLevel::CYCLE) {
        //Execution
        running = true;

        while(running) {
            if(clockEnabled) {
                clock.beforeLoopIteration();
            }

            executionManager.loop();

            if(clockEnabled) {
                clock.afterLoopIteration();
            }

            if(lms::extra::FILE_MONITOR_SUPPORTED && monitorEnabled
                    && monitor.hasChangedFiles()) {
                monitor.unwatchAll();
                parseConfig(LoadConfigFlag::ONLY_MODULE_CONFIG);
            }
        }
    }
}
/*
 * TODO suffix for config
 */
void Framework::parseConfig(LoadConfigFlag flag){

    logger.debug("parseConfig") << "EXTERNAL: " << externalDirectory
                                << std::endl << "CONFIGS: " << configsDirectory;

    std::string configPath = configsDirectory + "/";
    if(argumentHandler.argLoadConfiguration().empty()) {
        configPath += "framework_conf.xml";
    } else {
        configPath += argumentHandler.argLoadConfiguration() + ".xml";
    }

    parseFile(configPath, flag);
}

void Framework::parseFile(const std::string &file, LoadConfigFlag flag) {
    logger.debug("parseFile") << "Reading XML file: " << file;
    if(lms::extra::FILE_MONITOR_SUPPORTED && !monitor.watch(file)) {
        logger.error("parseFile") << "Could not monitor " << file;
    }

    std::ifstream ifs;
    ifs.open (file, std::ifstream::in);
    if(ifs.is_open()){
        pugi::xml_document doc;
        pugi::xml_parse_result result = doc.load(ifs);
        if (result){
            pugi::xml_node rootNode = doc.child("framework");

            if(flag != LoadConfigFlag::ONLY_MODULE_CONFIG) {
                // parse <execution> tag (or deprecated <executionManager>)
                parseExecution(rootNode);

                // parse <moduleToEnable> tag (or deprecated <modulesToLoad>)
                parseModulesToEnable(rootNode);
            }

            // parse all <module> tags
            parseModules(rootNode, file, flag);

            // parse all <include> tags
            parseIncludes(rootNode, file, flag);
        } else {
            logger.error() << "Failed to parse " << file << " as XML";
        }
    } else {
        logger.error() << "Failed to open XML file " << file;
    }
}

void Framework::parseExecution(pugi::xml_node rootNode) {
    pugi::xml_node execNode = rootNode.child("execution");

    if(!execNode) {
        execNode = rootNode.child("executionManager");

        if(execNode) {
            logger.warn("parseExecution")
                << "Found deprecated tag <executionManager>, use <execution> instead";
        } else {
            // do not parse anything
            return;
        }
    }

    pugi::xml_node threadPoolNode = execNode.child("maxThreadCount");

    if(threadPoolNode) {
        int maxThreads = atoi(threadPoolNode.child_value());
        executionManager.setMaxThreads(maxThreads);
        logger.info("parseExecution") << "Thread pool size: " << maxThreads;
        logger.info("parseExecution") << "Hardware Concurrency: " << std::thread::hardware_concurrency();
    }

    pugi::xml_node clockNode = execNode.child("clock");

    if(clockNode) {
        std::string clockUnit;
        std::int64_t clockValue = atoll(clockNode.child_value());

        pugi::xml_attribute enabledAttr = clockNode.attribute("enabled");
        pugi::xml_attribute unitAttr = clockNode.attribute("unit");

        if(enabledAttr) {
            clockEnabled = enabledAttr.as_bool();
        } else {
            // if no enabled attribute is given then the clock is considered
            // to be disabled
            clockEnabled = false;
        }

        if(unitAttr) {
            clockUnit = unitAttr.value();
        } else {
            logger.warn("parseExecution")
                << "Missing attribute unit=\"hz/ms/us\" for tag <clock>";
            clockEnabled = false;
        }

        if(clockEnabled) {
            if(clockUnit == "hz") {
                clock.cycleTime(extra::PrecisionTime::fromMicros(1000000 / clockValue));
            } else if(clockUnit == "ms") {
                clock.cycleTime(extra::PrecisionTime::fromMillis(clockValue));
            } else if(clockUnit == "us") {
                clock.cycleTime(extra::PrecisionTime::fromMicros(clockValue));
            } else {
                logger.error("parseConfig")
                    << "Invalid value for attribute unit in <clock>: "
                    << clockUnit;
                clockEnabled = false;
            }
        }
    }

    if(clockEnabled) {
        logger.info("parseConfig") << "Enabled clock with " << clock.cycleTime();
    } else {
        logger.info("parseConfig") << "Disabled clock";
    }

    pugi::xml_node configMonitorNode = execNode.child("configMonitor");

    if(configMonitorNode) {
        std::string configMonitorText = configMonitorNode.child_value();

        if(configMonitorText == "true") {
            monitorEnabled = true;
        } else if(configMonitorText == "false") {
            monitorEnabled = false;
        } else {
            logger.warn("parseConfig") << "Invalid value for <configMonitor>";
        }
    }

    if(monitorEnabled) {
        logger.info("parseConfig") << "Enabled config monitor";
    } else {
        logger.info("parseConfig") << "Disable config monitor";
    }

    pugi::xml_node profilingNode = execNode.child("profiling");

    if(profilingNode) {
        pugi::xml_attribute enabledAttr = profilingNode.attribute("enabled");

        if(enabledAttr) {
            executionManager.enableProfiling(enabledAttr.as_bool(false));
        } else {
            logger.error("parseConfig") << "Found <profiling /> tag without"
                                       << " enabled attribute";
        }
    }
}

void Framework::parseModulesToEnable(pugi::xml_node rootNode) {
    pugi::xml_node enableNode = rootNode.child("modulesToEnable");

    if(! enableNode) {
        enableNode = rootNode.child("modulesToLoad");

        if(enableNode) {
            logger.warn("parseModulesToEnable")
                << "Found deprecated tag <modulesToLoad>, use <modulesToEnable> instead";
        } else {
            // do not parse anything
            return;
        }
    }

    lms::logging::LogLevel defaultModuleLevel = lms::logging::SMALLEST_LEVEL;

    // get attribute "logLevel" of node <modulesToLoad>
    // its value will be the default for logLevel of <module>
    pugi::xml_attribute globalLogLevelAttr = enableNode.attribute("logLevel");
    if(globalLogLevelAttr) {
        defaultModuleLevel = lms::logging::levelFromName(globalLogLevelAttr.value());
    }

    for (pugi::xml_node moduleNode : enableNode.children("module")){
        //parse module content
        ModuleToLoad mod;
        mod.name = moduleNode.child_value();

        // get the attribute "logLevel"
        pugi::xml_attribute logLevelAttr = moduleNode.attribute("logLevel");
        if(logLevelAttr) {
            mod.logLevel = lms::logging::levelFromName(logLevelAttr.value());
        } else {
            mod.logLevel = defaultModuleLevel;
        }

        tempModulesToLoadList.push_back(mod);
    }
}

void Framework::parseModules(pugi::xml_node rootNode,
                             const std::string &currentFile,
                             LoadConfigFlag flag) {
    // parse all <module> nodes
    for (pugi::xml_node moduleNode : rootNode.children("module")) {

        Loader::module_entry module;
        std::map<std::string, type::ModuleConfig> configMap;

        module.name = moduleNode.child("name").child_value();
        logger.info("parseModules") << "Found def for module " << module.name;

        pugi::xml_node realNameNode = moduleNode.child("realName");

        if(realNameNode) {
            module.libpath = externalDirectory + "/modules/" +
                    realNameNode.child_value() + "/" +
                    Loader::getModulePath(realNameNode.child_value());
        } else {
            pugi::xml_node libpathNode = moduleNode.child("libpath");
            pugi::xml_node libnameNode = moduleNode.child("libname");

            std::string libname;
            if(libnameNode) {
                libname = Loader::getModulePath(libnameNode.child_value());
            } else {
                libname = Loader::getModulePath(module.name);
            }

            if(libpathNode) {
                // TODO better relative path here
                module.libpath = externalDirectory + "/modules/" +
                        libpathNode.child_value() + "/" + libname;
            } else {
                module.libpath = externalDirectory + "/modules/" + module.name
                        + "/" + libname;
            }
        }

        pugi::xml_node writePrioNode = moduleNode.child("writePriority");

        if(writePrioNode) {
            module.writePriority = atoi(writePrioNode.child_value());
        } else {
            module.writePriority = 0;
        }

        pugi::xml_node executionTypeNode = moduleNode.child("executionType");

        if(executionTypeNode) {
            std::string executionType = executionTypeNode.child_value();

            if(executionType == "ONLY_MAIN_THREAD") {
                module.executionType = Loader::module_entry::ONLY_MAIN_THREAD;
            } else if(executionType == "NEVER_MAIN_THREAD") {
                module.executionType = Loader::module_entry::NEVER_MAIN_THREAD;
            } else {
                logger.error("parseModule") << "Invalid value for executionType: "
                                            << executionType;
            }
        } else {
            module.executionType = Loader::module_entry::NEVER_MAIN_THREAD;
        }

        // parse all channel mappings
        for(pugi::xml_node mappingNode : moduleNode.children("channelMapping")) {
            pugi::xml_attribute fromAttr = mappingNode.attribute("from");
            pugi::xml_attribute toAttr = mappingNode.attribute("to");

            if(fromAttr && toAttr) {
                module.channelMapping[fromAttr.value()] = toAttr.value();
            } else {
                logger.warn("parseModules")
                    << "Tag <channelMapping> requires from and to attributes";
            }
        }

        // parse all config
        for(pugi::xml_node configNode : moduleNode.children("config")) {
            pugi::xml_attribute srcAttr = configNode.attribute("src");
            pugi::xml_attribute nameAttr = configNode.attribute("name");

            std::string name = "default";
            if(nameAttr) {
                name = nameAttr.value();
            }

            if(srcAttr) {
                std::string lconfPath = srcAttr.value();

                if(extra::isAbsolute(lconfPath)) {
                    lconfPath = configsDirectory + lconfPath;
                } else {
                    lconfPath = extra::dirname(currentFile) + "/" + lconfPath;
                }

                bool loadResult = configMap[name].loadFromFile(lconfPath);
                if(!loadResult) {
                    logger.error("parseModules") << "Tried to load "
                        << srcAttr.value() << " for " << module.name << " but failed";

                } else {
                    logger.info("parseModules") << "Loaded " << lconfPath;
                    if(lms::extra::FILE_MONITOR_SUPPORTED &&
                            !monitor.watch(lconfPath)) {
                        logger.error("parseModules") << "Failed to monitor "
                                                     << lconfPath;
                    }
                }
            } else {
                // if there was no src attribut then parse the tag's content
                parseModuleConfig(configNode, "", configMap[name]);
            }
        }

        for(std::pair<std::string, type::ModuleConfig> pair : configMap) {
            executionManager.getDataManager()
                .setChannel<type::ModuleConfig>(
                "CONFIG_" + module.name + "_" + pair.first, pair.second);
        }

        if(flag != LoadConfigFlag::ONLY_MODULE_CONFIG) {
            executionManager.addAvailableModule(module);
        }
    }
}

void Framework::parseModuleConfig(pugi::xml_node node, const std::string &key,
                                  type::ModuleConfig &config) {
    // if node has no children
    if(node.type() == pugi::node_pcdata) {
        //logger.debug("parseModules") << key << " = " << extra::trim(node.value());
        config.set<std::string>(key, extra::trim(node.value()));
    } else if(node.type() == pugi::node_element) {
        std::string newKey;

        for(pugi::xml_node subnode : node.children()) {
            if(key.empty()) {
                newKey = subnode.name();
            } else if(subnode.type() == pugi::node_element) {
                newKey = key + "." + subnode.name();
            } else {
                newKey = key;
            }
            parseModuleConfig(subnode, newKey , config);
        }
    }
}

void Framework::parseIncludes(pugi::xml_node rootNode,
                              const std::string &currentFile,
                              LoadConfigFlag flag) {
    for(pugi::xml_node includeNode : rootNode.children("include")) {
        pugi::xml_attribute srcAttr = includeNode.attribute("src");

        if(srcAttr) {
            logger.info("parseIncludes") << "Found include " << srcAttr.value();

            std::string includePath = srcAttr.value();
            if(extra::isAbsolute(includePath)) {
                // if absolute then start from configs dir
                includePath = configsDirectory + includePath;
            } else {
                // otherwise go from current file's directory
                includePath = extra::dirname(currentFile) + "/" + includePath;
            }
            parseFile(includePath, flag);
        } else {
            logger.error("Include tag has no src attribute");
        }
    }
}

Framework::~Framework() {
    logger.info() << "Removing Signal listeners";
    SignalHandler::getInstance()
            .removeListener(SIGINT, this)
            .removeListener(SIGSEGV, this);
}

void Framework::signal(int s) {
    switch (s) {
    case SIGINT:
        running = false;

        logger.warn() << "Terminating after next Cycle. Press CTRL+C again to terminate immediately";

        SignalHandler::getInstance().removeListener(SIGINT, this);

        break;
    case SIGSEGV:
        //Segmentation Fault - try to identify what went wrong;
        logger.error()
                << "######################################################" << std::endl
                << "                   Segfault Found                     " << std::endl
                << "######################################################";

        //In Case of Segfault while recovering - shutdown.
        SignalHandler::getInstance().removeListener(SIGSEGV, this);

        extra::printStacktrace();

        exit(EXIT_FAILURE);

        break;
    }
}
}

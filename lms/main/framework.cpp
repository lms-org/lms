#include <lms/framework.h>
#include <lms/executionmanager.h>
#include <pugixml.hpp>
#include <fstream>
#include <csignal>
#include <map>
#include <thread>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include "lms/extra/backtrace_formatter.h"
#include "lms/logger.h"
#include "lms/extra/time.h"
#include "lms/type/module_config.h"
#include "lms/extra/string.h"
#include "lms/xml_parser.h"
#include "lms/extra/colors.h"

namespace lms{

std::string Framework::externalDirectory = EXTERNAL_DIR;
std::string Framework::configsDirectory = CONFIGS_DIR;

Framework::Framework(const ArgumentHandler &arguments) :
    logger("lms::Framework"), argumentHandler(arguments), executionManager(),
    configMonitorEnabled(false) {

    logging::Context &ctx = logging::Context::getDefault();

    if(! arguments.argQuiet) {
        ctx.appendSink(new logging::ConsoleSink(std::cout));
    }

    if(! arguments.argLogFile.empty()) {
        ctx.appendSink(new logging::FileSink(arguments.argLogFile));
    }

    SignalHandler::getInstance()
            .addListener(SIGINT, this)
            .addListener(SIGSEGV, this);

    executionManager.profiler().enabled(argumentHandler.argProfiling);

    if(argumentHandler.argProfiling) {
        logger.info() << "Enable profiling";
    } else {
        logger.info() << "Disable profiling";
    }

    configMonitorEnabled = argumentHandler.argConfigMonitor;

    if(configMonitorEnabled) {
        logger.info() << "Enable config monitor";
    } else {
        logger.info() << "Disable config monitor";
    }

    executionManager.enabledMultithreading(argumentHandler.argMultithreaded);

    if(argumentHandler.argMultithreaded) {
        if(argumentHandler.argThreadsAuto) {
            executionManager.numThreadsAuto();
            logger.info() << "Multithreaded with " << executionManager.numThreads() << " threads (auto)";
        } else {
            executionManager.numThreads(argumentHandler.argThreads);
            logger.info() << "Multithreaded with " << executionManager.numThreads() << " threads";
        }
    } else {
        logger.info() << "Single threaded";
    }

    logger.info() << "RunLevel " <<  arguments.argRunLevel;

    //parse framework config
    if(arguments.argRunLevel >= RunLevel::CONFIG) {
        parseConfig(LoadConfigFlag::LOAD_EVERYTHING);
    }

    if(arguments.argRunLevel >= RunLevel::ENABLE) {
        // enable modules after they were made available
        logger.info() << "Start enabling modules";
        for(ModuleToLoad mod : modulesToLoadLists["default"]) {
            executionManager.enableModule(mod.name, mod.logLevel);
        }

        if(arguments.argRunLevel == RunLevel::ENABLE) {
            executionManager.getDataManager().printMapping();
            executionManager.validate();
            executionManager.printCycleList();
        }
    }

    if(arguments.argRunLevel >= RunLevel::CYCLE) {
        logger.info() << "Start running modules";

        ctx.filter(new logging::ThresholdFilter(arguments.argLoggingMinLevel));

        //Execution
        running = true;

        while(running) {
            clock.beforeLoopIteration();
            if(executionManager.profiler().enabled()){
                logger.time("totalTime");
            }
            executionManager.loop();
            if(executionManager.profiler().enabled()){
                logger.timeEnd("totalTime");
            }

            if(lms::extra::FILE_MONITOR_SUPPORTED && configMonitorEnabled
                    && configMonitor.hasChangedFiles()) {
                configMonitor.unwatchAll();
                modulesToLoadLists.clear();
                parseConfig(LoadConfigFlag::ONLY_MODULE_CONFIG);
                executionManager.fireConfigsChangedEvent();
            }

            for(const std::string &message : executionManager.messaging().
                receive("loadConfig")) {

                logger.info("loadConfig") << "START";

                ModuleList forDisable;

                // disable all modules that are not needed anymore
                for(std::shared_ptr<ModuleWrapper> mod : executionManager.getEnabledModules()) {
                    bool found = false;

                    for(const ModuleToLoad &loadMod : modulesToLoadLists[message]) {
                        if(mod->name == loadMod.name) {
                            found = true;
                            break;
                        }
                    }

                    if(! found) {
                        forDisable.push_back(mod);
                    }
                }

                // disable in reversed order
                std::reverse(forDisable.begin(), forDisable.end());

                for(std::shared_ptr<ModuleWrapper> mod : forDisable) {
                    executionManager.disableModule(mod->name);
                }

                logger.info("loadConfig") << "MID";

                for(const ModuleToLoad &mod : modulesToLoadLists[message]) {
                    executionManager.enableModule(mod.name, mod.logLevel);
                }

                executionManager.invalidate();

                executionManager.getDataManager().printMapping();

                logger.info("loadConfig") << "STOP";
            }
        }

        ctx.filter(nullptr);
        logger.info() << "Stopped";
    }
}
/*
 * TODO suffix for config
 */
void Framework::parseConfig(LoadConfigFlag flag){

    logger.debug("parseConfig") << "EXTERNAL: " << externalDirectory
                                << std::endl << "CONFIGS: " << configsDirectory;

    std::string configPath = configsDirectory + "/";
    if(argumentHandler.argLoadConfiguration.empty()) {
        configPath += "framework_conf.xml";
    } else {
        configPath += argumentHandler.argLoadConfiguration + ".xml";
    }

    parseFile(configPath, flag);
}

void Framework::parseFile(const std::string &file, LoadConfigFlag flag) {
    if(lms::extra::FILE_MONITOR_SUPPORTED && !configMonitor.watch(file)) {
        logger.error("parseFile") << "Could not monitor " << file;
    }

    std::ifstream ifs;
    ifs.open (file, std::ifstream::in);
    if(ifs.is_open()){
        pugi::xml_document doc;
        pugi::xml_parse_result result = doc.load(ifs);
        if (result){
            pugi::xml_node rootNode = doc.child("framework");
            lms::preprocessXML(rootNode, argumentHandler.argFlags);

            if(flag != LoadConfigFlag::ONLY_MODULE_CONFIG) {
                // parse <execution> tag (or deprecated <executionManager>)
                parseExecution(rootNode);
            }

            // parse <moduleToEnable> tag
            parseAllModulesToEnable(rootNode);

            // parse all <module> tags
            parseModules(rootNode, file, flag);

            // parse all <include> tags
            parseIncludes(rootNode, file, flag);
        } else {
            logger.error() << "Failed to parse " << file << " as XML: ";
            logger.error() << result.offset << " " << result.description();
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

    pugi::xml_node clockNode = execNode.child("clock");

    if(clockNode) {
        std::string clockUnit;
        std::int64_t clockValue = 0;

        pugi::xml_attribute enabledAttr = clockNode.attribute("enabled");
        pugi::xml_attribute unitAttr = clockNode.attribute("unit");
        pugi::xml_attribute valueAttr = clockNode.attribute("value");

        if(enabledAttr) {
            clock.enabled(enabledAttr.as_bool());
        } else {
            // if not enabled attribute is given then the clock is considered
            // to be disabled
            clock.enabled(false);
        }

        if(valueAttr) {
            clockValue = valueAttr.as_llong();
        } else {
            logger.error("parseExecution")
                << "Missing attribute value for tag <clock>";
            clock.enabled(false);
        }

        if(unitAttr) {
            clockUnit = unitAttr.value();
        } else {
            logger.warn("parseExecution")
                << "Missing attribute unit=\"hz/ms/us\" for tag <clock>";
            clock.enabled(false);
        }

        if(clock.enabled()) {
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
                clock.enabled(false);
            }
        }
    }

    if(clock.enabled()) {
        logger.info("parseConfig") << "Enabled clock with " << clock.cycleTime();
    } else {
        logger.info("parseConfig") << "Disabled clock";
    }
}

void Framework::parseAllModulesToEnable(pugi::xml_node rootNode) {
    for(pugi::xml_node node : rootNode.children("modulesToEnable")) {
        parseModulesToEnable(node, modulesToLoadLists);
    }
}

void Framework::parseModules(pugi::xml_node rootNode,
                             const std::string &currentFile,
                             LoadConfigFlag flag) {
    // parse all <module> nodes
    for (pugi::xml_node moduleNode : rootNode.children("module")) {

        std::shared_ptr<ModuleWrapper> module = std::make_shared<ModuleWrapper>();
        std::map<std::string, type::ModuleConfig> configMap;

        module->name = moduleNode.child("name").child_value();

        pugi::xml_node realNameNode = moduleNode.child("realName");

        if(realNameNode) {
            module->libpath = externalDirectory + "/modules/" +
                    realNameNode.child_value() + "/" +
                    Loader::getModulePath(realNameNode.child_value());
        } else {
            pugi::xml_node libpathNode = moduleNode.child("libpath");
            pugi::xml_node libnameNode = moduleNode.child("libname");

            std::string libname;
            if(libnameNode) {
                libname = Loader::getModulePath(libnameNode.child_value());
            } else {
                libname = Loader::getModulePath(module->name);
            }

            if(libpathNode) {
                // TODO better relative path here
                module->libpath = externalDirectory + "/modules/" +
                        libpathNode.child_value() + "/" + libname;
            } else {
                module->libpath = externalDirectory + "/modules/" + module->name
                        + "/" + libname;
            }
        }

        pugi::xml_node writePrioNode = moduleNode.child("writePriority");

        if(writePrioNode) {
            module->writePriority = atoi(writePrioNode.child_value());
        } else {
            module->writePriority = 0;
        }

        pugi::xml_node executionTypeNode = moduleNode.child("executionType");

        if(executionTypeNode) {
            std::string executionType = executionTypeNode.child_value();

            if(executionType == "ONLY_MAIN_THREAD") {
                module->executionType = ModuleWrapper::ONLY_MAIN_THREAD;
            } else if(executionType == "NEVER_MAIN_THREAD") {
                module->executionType = ModuleWrapper::NEVER_MAIN_THREAD;
            } else {
                logger.error("parseModule") << "Invalid value for executionType: "
                                            << executionType;
            }
        } else {
            module->executionType = ModuleWrapper::NEVER_MAIN_THREAD;
        }

        pugi::xml_node expectedRuntimeNode = moduleNode.child("expectedRuntime");

        if(expectedRuntimeNode) {
            module->expectedRuntime = extra::PrecisionTime::fromMicros(
                        atoi(expectedRuntimeNode.child_value()));
        } else {
            module->expectedRuntime = extra::PrecisionTime::ZERO;
        }

        // parse all channel mappings
        for(pugi::xml_node mappingNode : moduleNode.children("channelMapping")) {
            pugi::xml_attribute fromAttr = mappingNode.attribute("from");
            pugi::xml_attribute toAttr = mappingNode.attribute("to");

            if(fromAttr && toAttr) {
                module->channelMapping[fromAttr.value()] = toAttr.value();
            } else {
                logger.warn("parseModules")
                    << "Tag <channelMapping> requires from and to attributes";
            }
        }

        // parse all config
        for(pugi::xml_node configNode : moduleNode.children("config")) {
            pugi::xml_attribute srcAttr = configNode.attribute("src");
            pugi::xml_attribute nameAttr = configNode.attribute("name");
            pugi::xml_attribute userAttr = configNode.attribute("user");

            if(userAttr) {
                std::vector<std::string> allowedUsers =
                        lms::extra::split(std::string(userAttr.value()), ',');

                if(std::find(allowedUsers.begin(), allowedUsers.end(),
                             argumentHandler.argUser) == allowedUsers.end()) {
                    continue;
                }
            }

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
                        << srcAttr.value() << " for " << module->name << " but failed";

                } else {
                    if(lms::extra::FILE_MONITOR_SUPPORTED &&
                            !configMonitor.watch(lconfPath)) {
                        logger.error("parseModules") << "Failed to monitor "
                                                     << lconfPath;
                    }
                }
            } else {
                // if there was no src attribut then parse the tag's content
                parseModuleConfig(configNode, configMap[name], "");
            }
        }

        for(std::pair<std::string, type::ModuleConfig> pair : configMap) {
            executionManager.getDataManager()
                .setChannel<type::ModuleConfig>(
                "CONFIG_" + module->name + "_" + pair.first, pair.second);
        }

        if(flag != LoadConfigFlag::ONLY_MODULE_CONFIG) {
            executionManager.addAvailableModule(module);
        }
    }
}

void Framework::parseIncludes(pugi::xml_node rootNode,
                              const std::string &currentFile,
                              LoadConfigFlag flag) {
    for(pugi::xml_node includeNode : rootNode.children("include")) {
        pugi::xml_attribute srcAttr = includeNode.attribute("src");

        if(srcAttr) {
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
    SignalHandler::getInstance()
            .removeListener(SIGINT, this)
            .removeListener(SIGSEGV, this);
}

void Framework::signal(int s) {
    switch (s) {
    case SIGINT:
        running = false;

        SignalHandler::getInstance().removeListener(SIGINT, this);

        break;
    case SIGSEGV:
        //Segmentation Fault - try to identify what went wrong;
        std::cerr << std::endl << lms::extra::COLOR_RED
                << "######################################################" << std::endl
                << "                   Segfault Found                     " << std::endl
                << "######################################################" << std::endl
                << lms::extra::COLOR_WHITE;

        //In Case of Segfault while recovering - shutdown.
        SignalHandler::getInstance().removeListener(SIGSEGV, this);

        extra::printStacktrace();

        exit(EXIT_FAILURE);

        break;
    }
}
}

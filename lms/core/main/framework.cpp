#include <lms/framework.h>
#include <lms/executionmanager.h>
#include <pugixml.hpp>
#include <fstream>
#include <csignal>
#include <cstdlib>
#include "lms/extra/backtrace_formatter.h"
#include "lms/logging/log_level.h"
#include "lms/extra/time.h"
#include "unistd.h"
#include "lms/type/module_config.h"

namespace lms{

std::string Framework::externalDirectory = EXTERNAL_DIR;
std::string Framework::configsDirectory = CONFIGS_DIR;

Framework::Framework(const ArgumentHandler &arguments) :
    logger("FRAMEWORK", &rootLogger), argumentHandler(arguments), executionManager(rootLogger),
    clockEnabled(false), clock(rootLogger) {

    rootLogger.filter(std::unique_ptr<logging::LoggingFilter>(new logging::PrefixAndLevelFilter(
        arguments.argLoggingMinLevel(), arguments.argLoggingPrefixes())));

    SignalHandler::getInstance()
            .addListener(SIGINT, this)
            .addListener(SIGSEGV, this);

    logger.info() << "RunLevel " <<  arguments.argRunLevel();

    //parse framework config
    if(arguments.argRunLevel() >= RunLevel::CONFIG) {
        parseConfig();
    }

    if(arguments.argRunLevel() >= RunLevel::ENABLE) {
        // enable modules after they were made available
        logger.info() << "Start enabling modules";
        for(ModuleToLoad mod : tempModulesToLoadList) {
            executionManager.enableModule(mod.name, mod.logLevel);
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
        }
    }
}
/*
 * TODO suffix for config
 */
void Framework::parseConfig(){

    logger.debug("parseConfig") << "EXTERNAL: " << externalDirectory
                                << std::endl << "CONFIGS: " << configsDirectory;

    std::string configPath = configsDirectory + "/";
    if(argumentHandler.argLoadConfiguration().empty()) {
        configPath += "framework_conf.xml";
    } else {
        configPath += argumentHandler.argLoadConfiguration() + ".xml";
    }

    parseFile(configPath);
}

void Framework::parseFile(const std::string &file) {
    logger.debug("parseFile") << "Reading XML file: " << file;

    std::ifstream ifs;
    ifs.open (file, std::ifstream::in);
    if(ifs.is_open()){
        pugi::xml_document doc;
        pugi::xml_parse_result result = doc.load(ifs);
        if (result){
            pugi::xml_node rootNode = doc.child("framework");

            // parse <execution> tag (or deprecated <executionManager>)
            parseExecution(rootNode);

            // parse <moduleToEnable> tag (or deprecated <modulesToLoad>)
            parseModulesToEnable(rootNode);

            // parse all <module> tags
            parseModules(rootNode);

            // parse all <include> tags
            parseIncludes(rootNode);
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

    pugi::xml_node clockNode = execNode.child("clock");

    if(clockNode) {
        std::string clockUnit;
        std::int64_t clockValue = atoll(clockNode.child_value());

        pugi::xml_attribute enabledAttr = clockNode.attribute("enable");
        pugi::xml_attribute unitAttr = clockNode.attribute("unit");

        if(enabledAttr) {
            clockEnabled = enabledAttr.as_bool();
        } else {
            // if no enabled attribute is given then the clock is considered
            // to be enabled
            clockEnabled = true;
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

void Framework::parseModules(pugi::xml_node rootNode) {
    // parse all <module> nodes
    for (pugi::xml_node moduleNode : rootNode.children("module")) {

        Loader::module_entry module;
        type::ModuleConfig config;

        module.name = moduleNode.child("name").child_value();
        logger.info("parseModules") << "Found def for module " << module.name;

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
            module.libpath = externalDirectory + "/modules/" + module.name + "/"
                + libname;
        }

        // parse all channel mappings
        for(pugi::xml_node mappingNode : moduleNode.children("channelMapping")) {
            pugi::xml_attribute fromAttr = mappingNode.attribute("from");
            pugi::xml_attribute toAttr = mappingNode.attribute("to");

            if(fromAttr && toAttr) {
                module.channelMapping[fromAttr.value()] = toAttr.value();
                logger.warn("parseModule") << fromAttr.value() << " -> " << toAttr.value();
            } else {
                logger.warn("parseModules")
                    << "Tag <channelMapping> requires from and to attributes";
            }
        }

        // parse all config
        for(pugi::xml_node configNode : moduleNode.children("config")) {
            pugi::xml_attribute srcAttr = configNode.attribute("src");

            if(srcAttr) {
                bool loadResult = config.loadFromFile(configsDirectory
                                                      + "/" + srcAttr.value());
                if(!loadResult) {
                    logger.error("parseModules") << "Tried to load "
                        << srcAttr.value() << " for " << module.name << " but failed";

                }
            } else {
                // if there was no src attribut then parse the tag's content
                for (pugi::xml_node configPropNode: configNode.children()) {
                    logger.debug("parseModules") << configPropNode.name();
                    config.set(configPropNode.name(),
                                     std::string(configPropNode.child_value()));
                }
            }
        }

        executionManager.getDataManager()
            .setChannel<type::ModuleConfig>("CONFIG_" + module.name, config);

        executionManager.addAvailableModule(module);
    }
}

void Framework::parseIncludes(pugi::xml_node rootNode) {
    for(pugi::xml_node includeNode : rootNode.children("include")) {
        pugi::xml_attribute srcAttr = includeNode.attribute("src");

        if(srcAttr) {
            logger.info("parseIncludes") << "Found include " << srcAttr.value();
            parseFile(configsDirectory + "/" + srcAttr.value());
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

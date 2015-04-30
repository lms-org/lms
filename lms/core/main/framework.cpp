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

    //parse framework config
    parseConfig();

    // enable modules after they were made available
    logger.info() << "Start enabling modules";
    for(ModuleToLoad mod : tempModulesToLoadList) {
        executionManager.enableModule(mod.name, mod.logLevel);
    }

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
/*
 * TODO suffix for config
 */
void Framework::parseConfig(){

    logger.debug("parseConfig") << "EXTERNAL: " << externalDirectory
                                << std::endl << "CONFIGS: " << configsDirectory;

    std::string configPath = configsDirectory + "/";
    if(argumentHandler.argLoadConfiguration().size() == 0){
        configPath +="framework_conf.xml";
    }else{
        configPath +="framework_conf_"+argumentHandler.argLoadConfiguration()+".xml";
    }

    parseFile(configPath);
}

void Framework::parseFile(const std::string &file) {
    std::ifstream ifs;
    ifs.open (file, std::ifstream::in);
    if(ifs.is_open()){
        pugi::xml_document doc;
        pugi::xml_parse_result result = doc.load(ifs);
        if (result){
            pugi::xml_node rootNode =doc.child("framework");
            pugi::xml_node tmpNode;
            //parse executionManager-stuff
            tmpNode = rootNode.child("execution");

            if(tmpNode) {
                parseExecution(tmpNode);
            }
            //TODO set values for executionManager

            //Start modules
            tmpNode = rootNode.child("modulesToLoad");

            lms::logging::LogLevel defaultModuleLevel = lms::logging::SMALLEST_LEVEL;

            // get attribute "logLevel" of node <modulesToLoad>
            // its value will be the default for logLevel of <module>
            for(pugi::xml_attribute_iterator attrIt = tmpNode.attributes_begin();
                attrIt != tmpNode.attributes_end(); ++attrIt) {

                if(std::string("logLevel") == attrIt->name()) {
                    defaultModuleLevel = lms::logging::levelFromName(attrIt->value());
                }
            }

            for (pugi::xml_node_iterator it = tmpNode.begin(); it != tmpNode.end(); ++it){
                //parse module content
                std::string moduleName = it->child_value();

                // get the attribute "logLevel"
                lms::logging::LogLevel level = defaultModuleLevel;
                for(pugi::xml_attribute_iterator attrIt = it->attributes_begin();
                    attrIt != it->attributes_end(); ++attrIt) {

                    if(std::string("logLevel") == attrIt->name()) {
                        level = lms::logging::levelFromName(attrIt->value());
                    }
                }

                ModuleToLoad mod;
                mod.name = moduleName;
                mod.logLevel = level;
                tempModulesToLoadList.push_back(mod);
            }

            parseModules(rootNode);

            parseIncludes(rootNode);
        }else{
            logger.error() << "Failed to parse framework_config.xml as XML";
        }
    } else {
        logger.error() << "Failed to open framework_config: " << file;
    }
}

void Framework::parseExecution(pugi::xml_node execNode) {
    pugi::xml_node clockNode = execNode.child("clock");

    if(clockNode) {
        std::string clockUnit;
        std::int64_t clockValue = atoll(clockNode.child_value());

        for(pugi::xml_attribute attr : clockNode.attributes()) {

            if(std::string("enabled") == attr.name()) {
                clockEnabled = attr.as_bool();
            } else if(std::string("unit") == attr.name()) {
                clockUnit = attr.value();
            }
        }

        if(clockEnabled) {
            if(clockUnit == "hz") {
                clock.cycleTime(extra::PrecisionTime::fromMicros(1000000 / clockValue));
            } else if(clockUnit == "ms") {
                clock.cycleTime(extra::PrecisionTime::fromMillis(clockValue));
            } else if(clockUnit == "us") {
                clock.cycleTime(extra::PrecisionTime::fromMicros(clockValue));
            } else {
                logger.error("parseConfig") << "Invalid clock unit: " << clockUnit;
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

        // parse all config
        for(pugi::xml_node configNode : moduleNode.children("config")) {
            bool hasSrcAttr = false;

            for (pugi::xml_attribute attr: configNode.attributes()) {
                if(std::string("src") == attr.name()) {
                    bool loadResult = config.loadFromFile(configsDirectory
                                                          + "/" + attr.value());
                    if(!loadResult) {
                        logger.error("parseModules") << "Tried to load "
                            << attr.value() << " for " << module.name << " but failed";

                    }
                    hasSrcAttr = true;
                    break;
                }
            }

            // if there was no src attribut then parse the tag's content
            if(!hasSrcAttr) {
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
        bool hasSrcAttr = false;
        for (pugi::xml_attribute attr: includeNode.attributes()) {
            if(std::string("src") == attr.name()) {
                hasSrcAttr = true;
                logger.info("parseIncludes") << "Found include " << attr.value();
                parseFile(configsDirectory + "/" + attr.value());
            }
        }

        if(! hasSrcAttr) {
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

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

namespace lms{

Framework::Framework(const ArgumentHandler &arguments) :
    logger("FRAMEWORK", &rootLogger), argumentHandler(arguments), executionManager(rootLogger),
    clockEnabled(false), clock(rootLogger) {

    rootLogger.filter(std::unique_ptr<logging::LoggingFilter>(new logging::PrefixAndLevelFilter(
        arguments.argLoggingMinLevel(), arguments.argLoggingPrefixes())));

    SignalHandler::getInstance()
            .addListener(SIGINT, this)
            .addListener(SIGSEGV, this);

    //load all Availabel Modules
    executionManager.loadAvailabelModules();

    //parse framework config
    parseConfig();
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

    std::string configPath = Framework::programDirectory()+"configs/";
    if(argumentHandler.argLoadConfiguration().size() == 0){
        configPath +="framework_conf.xml";
    }else{
        configPath +="framework_conf_"+argumentHandler.argLoadConfiguration()+".xml";
    }
    std::ifstream ifs;
    ifs.open (configPath, std::ifstream::in);
    if(ifs.is_open()){
        pugi::xml_document doc;
        pugi::xml_parse_result result = doc.load(ifs);
        if (result){
            pugi::xml_node rootNode =doc.child("framework");
            pugi::xml_node tmpNode;
            //parse executionManager-stuff
            tmpNode = rootNode.child("executionManager");

            pugi::xml_node clockNode = tmpNode.child("clock");

            if(clockNode) {
                std::string clockUnit;
                std::int64_t clockValue = atoll(clockNode.child_value());

                for(pugi::xml_attribute_iterator attrIt = clockNode.attributes_begin();
                    attrIt != clockNode.attributes_end(); ++attrIt) {

                    if(std::string("enabled") == attrIt->name()) {
                        clockEnabled = attrIt->as_bool();
                    } else if(std::string("unit") == attrIt->name()) {
                        clockUnit = attrIt->value();
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

            logger.info() << "Start enabling modules";
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

                executionManager.enableModule(moduleName, level);
            }
        }else{
            logger.error() << "Failed to parse framework_config.xml as XML";
        }
    }else{
        logger.error() << "Failed to open framework_config: " << configPath;


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

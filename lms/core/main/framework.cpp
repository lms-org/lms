#include <lms/framework.h>
#include <lms/executionmanager.h>
#include <pugixml.hpp>
#include <fstream>
#include <csignal>
#include <cstdlib>
#include "lms/extra/backtrace_formatter.h"
#include "unistd.h"

namespace lms{

Framework::Framework(const ArgumentHandler &arguments) :
    logger("FRAMEWORK", &rootLogger), argumentHandler(arguments), executionManager(rootLogger) {

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
        executionManager.loop();
//        if(usleep(4000*1000) == -1) {
//            logger.error("sleep") << errno;
//            usleep(4000*1000);
//        }
    }
}
/*
 * TODO suffix for config
 */
void Framework::parseConfig(){
    std::string configPath = Framework::programDirectory()+"configs/framework_conf.xml";
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
            //TODO set values for executionManager

            //Start modules
            tmpNode = rootNode.child("modulesToLoad");
            logger.info() << "Start enabling modules";
            for (pugi::xml_node_iterator it = tmpNode.begin(); it != tmpNode.end(); ++it){
                //parse module content
                std::string moduleName = it->child_value();
                executionManager.enableModule(moduleName);
            }
        }else{
            logger.error() << "Failed to parse framework_config.xml as XML";
        }
    }else{
        logger.error() << "Failed to open framework_config.xml";


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

        extra::BacktraceFormatter::print();

        exit(EXIT_FAILURE);

        break;
    }
}
}

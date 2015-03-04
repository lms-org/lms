#include <core/framework.h>
#include <core/executionmanager.h>
#include <pugixml.hpp>
#include <fstream>
#include "backtrace_formatter.h"

namespace lms{

Framework::Framework(const ArgumentHandler &arguments) :
    argumentHandler(arguments) {

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
        //usleep(9000*1000);
    }
}

void Framework::parseConfig(){
    std::cout<<"PATH :" + Framework::programDirectory() << std::endl;
    std::cout <<"PARSE FRAMEWORK CONFIG: "+Framework::programDirectory()+"configs/framwork_conf.xml"<<std::endl;
    std::ifstream ifs;
    ifs.open (Framework::programDirectory()+"configs/framework_config.xml", std::ifstream::in);
    if(ifs.is_open()){
        pugi::xml_document doc;
        pugi::xml_parse_result result = doc.load(ifs);
        if (result){
            pugi::xml_node rootNode =doc.child("framework");
            pugi::xml_node tmpNode;
            //parse executionManager-stuff
            tmpNode = rootNode.child("executionManager");

            tmpNode = rootNode.child("modulesToLoad");

            std::cout <<"START ENABLING MODULES: "<<std::endl;
            for (pugi::xml_node_iterator it = tmpNode.begin(); it != tmpNode.end(); ++it){
                //parse module content
                std::string moduleName = it->value();
                std::cout <<"MODULEEEEEEEEEEE: "+ moduleName<<std::endl;
                executionManager.enableModule(moduleName);
            }
        }else{
            //TODO errorhandling
            std::cout <<"FAILED TO READ CONFIG: "<<std::endl;
        }
    }else{
        std::cout <<"FAILED TO OPEN FRAMEWORK_FILE_CONFIG: "<<std::endl;

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

        BacktraceFormatter::print();
        break;
    }
}
}

#include <core/framework.h>
#include <core/executionmanager.h>
#include <pugixml.hpp>
#include <fstream>
#include <string>
#include <csignal>
#include <climits>
#include <cstring>
#include "backtrace_formatter.h"
#include "unistd.h"

#include <sys/stat.h>

#ifdef _WIN32
    #include <Windows.h>
#endif

namespace lms{

Framework::Framework(const ArgumentHandler &arguments) :
    logger("FRAMEWORK", &rootLogger), argumentHandler(arguments), executionManager(rootLogger) {

    rootLogger.filter(std::unique_ptr<logging::LoggingFilter>(new logging::PrefixAndLevelFilter(
        arguments.loggingMinLevel(), arguments.loggingPrefixes())));

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

        BacktraceFormatter::print();

        exit(1);

        break;
    }
}

std::string Framework::programDirectory(){
    static std::string directory;
    if(directory.empty()) {
#ifdef _WIN32
        HMODULE hModule = GetModuleHandleW(NULL);
        WCHAR path[MAX_PATH];
        GetModuleFileNameW(hModule, path, MAX_PATH);
        //wide char array

        //convert from wide char to narrow char array
        char ch[260];
        char DefChar = ' ';
        WideCharToMultiByte(CP_ACP,0,path,-1, ch,260,&DefChar, NULL);

        //A std:string  using the char* constructor.
        std::string ss(ch);
        directory = ss;
#else
        char path[PATH_MAX];
        memset (path, 0, PATH_MAX);
        if (readlink("/proc/self/exe", path, PATH_MAX) == -1) {
            perror("readlink failed");
            exit(1);
        }
        //get programmdirectory
        // TODO optimize this a bit
        directory = path;
        directory = directory.substr(0, directory.rfind("/"));
        directory = directory.substr(0, directory.rfind("/"));
        directory = directory + "/";
#endif
    }
    
    //std::cout << "ProgramDirectory: " << directory << std::endl;
    return directory;
}

}

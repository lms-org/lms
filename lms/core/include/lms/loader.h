#ifndef LMS_LOADER_H
#define LMS_LOADER_H
/**
  *TODO: use C++ for io
  *
  */
#include <list>
#include <string>
#include <map>

#include "pugixml.hpp"
#include <lms/logger.h>

namespace lms {

class Module;
/**
 * @brief The Loader class used to load Modules
 */
class Loader {
public:
    explicit Loader(logging::Logger &rootLogger);
    /**
     * @brief The module_entry struct
     * used to store available modules
     */
	struct module_entry {
        /**
         *name of the module
         */
		std::string name;
        /**
         * @brief libname name of the binary (without suffix or lib) if it's empty, the name will be used to open the library
         */
        std::string libpath;
        /**
         * @brief localPathToModule path from external/ to the directory containing the .so or .dll file
         */
        std::string localPathToModule;
        /**
         * @brief localPathToConfigs TODO not used yet?!
         */
        std::string localPathToConfigs;
        /**
         * @brief stringMapping used to store string mapping. For examples it's used to get different dataChannel in modules that share one binary
         */
        std::map<std::string, std::string> stringMapping;

        std::vector<std::string> configPaths;
    };
    typedef std::list<Loader::module_entry> moduleList;

    /**
     * @brief load
     * @return the instance of the module
     */
    Module* load (const module_entry &);
    //Doesnt needed at all
    void unload(Module*);

    /**
     * @brief getModulePath
     * @param localPathToModule the path from the modules folder to the module folder that contains the so file
     * @param libname the name of the binary (How it's written in CMakeLists.txt
     * @return the ABSOLUTE path of the shared library file (.so)
     */
    static std::string getModulePath(const std::string &libname);
private:

    logging::ChildLogger logger;

    /**
     * @brief checkModule checks if the module could be loaded
     * @param path
     * @return true if module is fine
     */
    bool checkModule(const char* path);

    /**
     * @brief pathToModules path from programm_directory to the modules folder
     */
    std::string pathToModules;
};

}  // namespace lms

#endif /* LMS_LOADER_H */

#ifndef LMS_LOADER_H
#define LMS_LOADER_H
/**
  *TODO: use C++ for io
  *
  */
#include <list>
#include <string>

#include <core/logger.h>

namespace lms{
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
        std::string libname;
        /**
         * @brief localPathToModule path from external/ to the directory containing the .so or .dll file
         */
        std::string localPathToModule;
        /**
         * @brief localPathToConfigs TODO not used yet?!
         */
        std::string localPathToConfigs;
    };
    typedef std::list<Loader::module_entry> moduleList;
    /**
     * @brief getModules get all modules that are inside the external/modules folder
     * @return
     */
    moduleList getModules();
    /**
     * @brief load
     * @return the instance of the module
     */
    Module* load (const module_entry &);
    //Doesnt needed at all
    void unload(Module*);
private:

    logging::ChildLogger logger;

    /**
     * @brief getModulePath
     * @param localPathToModule the path from the modules folder to the module folder that contains the so file
     * @param libname the name of the binary (How it's written in CMakeLists.txt
     * @return the ABSOLUTE path of the shared library file (.so)
     */
    std::string getModulePath(const std::string &localPathToModule, const std::string &libname);
    /**
     * @brief checkModule checks if the module could be loaded
     * @param path
     * @return true if module is fine
     */
    bool checkModule(const char* path);
    /**
     * @brief parseLoadConfig Adds modules defined in the config to the list if they exist
     * @param moduleDirectoryName That folderName: external/modules/[moduleDirectoryName]
     * @param configPath doesn't have to exist
     * @param list list to add the module_entry
     */
    void handleLoadConfig(const std::string &configPath, const std::string &moduleFolderName,
        moduleList& list);
    /**
     * @brief pathToModules path from programm_directory to the modules folder
     */
    std::string pathToModules;
};
}
#endif

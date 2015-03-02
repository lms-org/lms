#ifndef LOADER_H
#define LOADER_H
/**
  *TODO: use C++ for io
  *
  */
#include <list>
#include <string>

namespace lms{
class Module;

class Loader {
public:
	explicit Loader();
    /**
     * @brief The module_entry struct
     * used to store available modules
     */
	struct module_entry {
		std::string name;
        std::string localPathToModule;
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
	char stringbuffer[1024];
    /**
     * @brief getModulePath
     * @param localPathToModule the path from the modules folder to the module folder that contains the so file
     * @param moduleName the name of the module (How it's written in CMakeLists.txt
     * @return the ABSOLUTE path of the shared library file (.so)
     */
    std::string getModulePath(std::string localPathToModule, std::string moduleName);
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
    void handleLoadConfig(std::string configPath,std::string moduleFolderName,moduleList& list);
    /**
     * @brief pathToModules path from programm_directory to the modules folder
     */
    std::string pathToModules;
};
}
#endif

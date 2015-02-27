#ifndef LOADER_H
#define LOADER_H
/**
  *TODO: use C++ for
  *
  */
#include <list>
#include <string>

namespace lms{
class Module;

class Loader {
	std::string programm_directory;
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
    bool checkModule(const char* path);
    std::string pathToModules;
};
}
#endif

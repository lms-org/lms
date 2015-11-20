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
#include <lms/module_wrapper.h>

namespace lms {

class Module;
/**
 * @brief The Loader class used to load Modules
 */
class Loader {
public:
    Loader();

    /**
     * @brief load
     * @return the instance of the module
     */
    void load (ModuleWrapper *);
    //Doesnt needed at all
    void unload(ModuleWrapper *entry);

    /**
     * @brief getModulePath
     * @param localPathToModule the path from the modules folder to the module folder that contains the so file
     * @param libname the name of the binary (How it's written in CMakeLists.txt
     * @return the ABSOLUTE path of the shared library file (.so)
     */
    static std::string getModulePath(const std::string &libname);

    void addModulePath(std::string const& path, int recursion = 0);
private:
    logging::Logger logger;

    /**
     * @brief checkModule checks if the module could be loaded
     * @param path
     * @return true if module is fine
     */
    bool checkSharedLibrary(const std::string &libpath);

    std::map<std::string, std::string> m_pathMapping;
};

}  // namespace lms

#endif /* LMS_LOADER_H */

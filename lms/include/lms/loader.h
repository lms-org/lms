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
         * @brief Name of the module
         */
		std::string name;

        /**
         * @brief full, absolute name of the shared library that shall be loaded
         * if the module is enabled
         */
        std::string libpath;

        /**
         * @brief Used for transparent datachannel mapping. Maps requested
         * datachannels to the real ones.
         */
        std::map<std::string, std::string> channelMapping;

#ifdef _WIN32
        // TODO pointer to open shared library or something similar
#else
        /**
         * @brief Handle that is returned by dlopen and is needed by dlclose.
         */
        void *dlHandle;
#endif
    };
    typedef std::list<Loader::module_entry> moduleList;

    /**
     * @brief load
     * @return the instance of the module
     */
    Module* load (module_entry &);
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
    bool checkSharedLibrary(const std::string &libpath);
};

}  // namespace lms

#endif /* LMS_LOADER_H */

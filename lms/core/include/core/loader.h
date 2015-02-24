#ifndef LOADER_H
#define LOADER_H

#include <list>
#include <string>

class Shared_Base;

class Loader {
	std::string programm_directory;
public:
	explicit Loader();
	struct module_entry {
		std::string name;
		std::string module;
        //brauchen wir glaube ich nicht mehr
		std::string place;
	};
	typedef std::list<module_entry> module_list;
    module_list getModules();
	Shared_Base* load (const module_entry &);
	void unload(Shared_Base*);
private:
	char stringbuffer[1024];
	char* make_filename(const char* module, const char* place);
    /**
     * @brief Adds the place to the programm-directory
     * @param buffer
     * @param place
     * @return
     */
	char*  make_searchpath(char* buffer, const char* place);
	bool check_library(std::string filename);
};

#endif

#ifndef SHARED_DATAMANAGER_H
#define SHARED_DATAMANAGER_H

#include <deque>
#include <map>
#include <vector>
#include <string>

#include <core/handle.h>
#include <core/configfile.h>
#include <core/configurationmanager.h>

#define MEMORY_CHECK

class Framework;
class Shared_Base;
//Für was die classe?
class Logger;
//das wohl eher ins framework und daten an den data-manager übergeben
class ConfigurationManager;

namespace Access {

/**
 * @brief The Operation enum used to detect, which module wants to read, write or modify.
 * There can only be one WRITE-Module per channel
 */
enum Operation {
    READ,MODIFY, WRITE
};
}

class DataManager
{
    friend class Logger;
    friend class GenericHandle;
    friend class Framework;
public:
    // TODO wiese kommandozeilen argumente?
    DataManager();

    // TODO wieso virtual
    virtual ~DataManager();

    struct info_storage {
        int length;
        int id;
        GenericHandle *handle;
        bool registered;
        std::deque<std::string> acquired;
        std::string registerer;
    };
    const info_storage& get_channel_info(const std::string& name) {
        return retrieval[name].iterator->second;
    }
    
    const info_storage& get_channel_info(int handle) {
        return retrieval[memory[handle].name].iterator->second;
    }
    
    const info_storage& get_channel_info(GenericHandle *handle) {
         return get_channel_info(handle->getHandle());
    }
//TODO: nur noch eine methode für register/require etc

    /* Generate a Channel for the given Information */
    template <typename _T>
    Handle<_T>* register_channel(const char *name, Access::Operation op, size_t size = 0) {
        Handle<_T>* handle = acquire_channel<_T>(name, op);

        // TODO idiotencheck; wenn size != 0 dann einfach setzen
        size = std::max(sizeof(_T), size);

        register_new_channel(handle, name, size);
        return handle;
    }

    template <typename _T>
    Handle<_T> *acquire_channel(const char* name, Access::Operation op) {
        auto stor = acquire_channel_subscription(name, op);

        if (stor.handle == NULL) {
            stor.handle = new Handle<_T>(this, stor.id);
        }
        return (Handle<_T>*)stor.handle;
    }

    // UNUSED
    template <typename _T>
    Handle<_T>* get_channel_handle(const char* name) {
        return (Handle<_T>*)channel_name_to_handle(name);
    }

    // lädt config datei aus ordner
    ConfigFile *config(const char* configName);
    ConfigurationManager *cfgMgr() { return &conf_mgr; }

    //TODO: auch blöd so, was anderes überlegen
    std::string getConfigSuffix(std::string wished) {
        if (wished != "") return wished;
        else if (load_configuration != "")
            return load_configuration;
        else return "default";
    }
    void setActiveModule(Shared_Base* sb) {
        activeModule = sb;
    }

    /**
     * @brief Print data channels and the corresponding modules
     * that can read or write in those channels.
     */
    void print_mapping();

    /*
    //TODO kommt in einen state-handler
    bool rc() {
        return is_rc;
    }
    void setRC(bool state) {
        is_rc = state;
    }
    //das kommt auch in den state-Handler
	void print_state();
    */

private:
    // TODO hä?
    GenericHandle *channel_name_to_handle(const std::string& name) {
        auto it = retrieval.find(name);

        if (it == retrieval.end()) {
            return NULL;
        }
        return it->second.iterator->second.handle;
    }

    /**
     * @brief Copy data into handle's memory.
     */
    void set_channel(int channel_handle, void* data, int length = -1);

    /**
     * @brief get_channel Get memory and length of the memory section of
     * the given handle and return them in parameters data and length.
     * @param channel_handle
     * @param data output parameter
     * @param length output parameter
     * @return always true ;)
     */
    bool get_channel(int channel_handle, void** data, int *length = NULL);

    // TODO klein schreiben
    static int MemoryDelimiter;

    std::string load_configuration;

    // TODO was?
    char* data;
    int data_size;

    typedef std::deque<std::pair<std::string, info_storage> > management_array;
    management_array management;

    void parse_arguments(char*const * argv, const int argc);
    // TODO was ist das?
    struct storage_access {
        storage_access(management_array::iterator it, Access::Operation op) :
            iterator(it), operation(op) { }
        storage_access() { }
        management_array::iterator iterator;
        Access::Operation operation;
    };
    typedef std::map<std::string, storage_access> management_map;
    management_map retrieval;

    struct memory_layout {
        char* start;
        int length;
        int id;
        std::string name;
    };
    std::vector<memory_layout> memory;
    Shared_Base *activeModule;

    void initialize_data();
    //Das umbennen oder löschen
    void destroy_data();

    //Ins framework
    ConfigurationManager conf_mgr;

    // TODO in ein modul, nicht in core
    //bool is_rc;

    void register_new_channel(GenericHandle *handle, std::string name, size_t length);
    info_storage& acquire_channel_subscription(const char* name, Access::Operation op);

};

#endif

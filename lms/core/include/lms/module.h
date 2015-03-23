#ifndef LMS_MODULE_H
#define LMS_MODULE_H

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <lms/loader.h>
#include <lms/type/module_config.h>
#include <lms/messaging.h>

namespace lms {

class DataManager;
/**
 * @brief The Module class Module
 */
class Module {
public:
    Module(): logger("", nullptr), m_datamanager(nullptr),
        m_messaging(nullptr), priority(0) { }
    virtual ~Module() { }
	
    std::string getName() const;
    /**
     * called by the framework itself at module-creation
    */
    bool initializeBase(DataManager* datamanager, Messaging *messaging,
                        Loader::module_entry& loaderEntry, logging::Logger *rootLogger);

    int getPriority() const;

    /**
     * TODO
     * @brief initialize
     * @param d
     * @return
     */
    virtual bool initialize() = 0;
	virtual bool deinitialize() = 0;
	virtual bool cycle() = 0;

    /**
     * @brief reset called by the framework
     * The following implementation is fully backwards compatible:
     * Modules can override reset if they can implement it
     * Don't forget to override isResettable() if you have a reset method!
     */
    virtual void reset() {}

    /**
     * @brief isResettable
     * If a module overrides reset() then isResettable() should be overriden to return true
     * @return
     */
    virtual bool isResettable() { return false; }

    /**
     * @brief getDataChannelMapping
     * @param mapFrom
     * @return the mapping for the given key or the key itself if it has no mapping for it
     */
    std::string getStringMapping(std::string mapFrom);

protected:
    DataManager* datamanager() const { return m_datamanager; }
    Messaging* messaging() const { return m_messaging; }
    logging::ChildLogger logger;
    const type::ModuleConfig* getConfig();
private:
    Loader::module_entry loaderEntry;
    DataManager* m_datamanager;
    Messaging* m_messaging;
    int priority;
};

}  // namespace lms

#endif /* LMS_MODULE_H */

#ifndef LMS_INTERNAL_WRAPPER_H
#define LMS_INTERNAL_WRAPPER_H

#include <string>

#include "lms/life_cycle.h"

namespace lms {
namespace internal {

class Wrapper {
public:
    virtual ~Wrapper();

    std::string lib() const;
    void lib(std::string const& lib);

    std::string clazz() const;
    void clazz(std::string const& clazz);

    std::string name() const;
    void name(std::string const& name);

    /**
     * @brief May be called by lms::internal::Loader to
     * set the Module or Service object.
     *
     * @param instance non-null Module or Service object
     */
    virtual void load(LifeCycle* instance) =0;

    /**
     * @brief Free and delete the internal Module or Service
     * object that may be set previously by load().
     */
    virtual void unload() =0;

    /**
     * @brief Name of the C function exported in
     * so/dylib/dll files to create Module or
     * Service objects.
     */
    virtual std::string interfaceFunction() const =0;
private:
    /**
     * @brief Name of the module or service.
     */
    std::string m_name;

    /**
     * @brief Basename of the so/dylib/dll file to be loaded.
     */
    std::string m_lib;

    /**
     * @brief Name of the class that will be loaded from the lib.
     */
    std::string m_class;
};

}  // namespace internal
}  // namespace lms

#endif // LMS_INTERNAL_WRAPPER_H

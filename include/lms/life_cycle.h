#ifndef LMS_LIFE_CYCLE_H
#define LMS_LIFE_CYCLE_H

namespace lms {

/**
 * @brief Base class of Module and Service.
 *
 * Defines the common life-cycle states of
 * modules and services as pure virtual methods.
 */
class LifeCycle {
public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~LifeCycle() {}

    /**
     * @brief This object is going to be alive.
     *
     * Allocate memory, open file descriptors or do any
     * other initialization stuff.
     *
     * No other life-cycle methods may be called before
     * init().
     *
     * @return true if succesful, false if not startable
     */
    virtual bool init() = 0;

    /**
     * @brief This object will be destroyed.
     *
     * Free memory, close file descriptors or do any
     * other cleanup stuff.
     *
     * No other life-cycle methods may be called after
     * destroy().
     */
    virtual void destroy() = 0;

    /**
     * @brief Any config might have changed by the user.
     */
    virtual void configsChanged() = 0;
};

} // namespace lms

#endif // LMS_LIFE_CYCLE_H

#ifndef FRAMEWORK_H
#define FRAMEWORK_H

#include <core/datamanager.h>
#include <core/signalhandler.h>
#include <core/argumentparser.h>
#include <core/executionmanager.h>
#include <core/logger.h>

/**
 * @brief Main entry point into the lightweight module system.
 *
 * Create an object of this class and you will start the framwork.
 */
class Framework : public SignalHandler::Listener {
public:
    /**
     * @brief Instantiate the framework, initiate datamanager and
     * execution manager and start the main loop.
     *
     * @param arguments parsed command line arguments
     */
    Framework(const ArgumentHandler& arguments);

    /**
     * @brief Destroy datamanager and execution manager.
     */
    ~Framework();
protected:
    /**
     * @brief Override this method to set custom Managers!
     */
    virtual void initManagers();
private:
    bool running;

    void signal(int s);

    DataManager *dataManager;
    ExecutionManager *executionManager;
    ArgumentHandler argumentHandler;

    Sink *sink;
    RootLogger *logger;
};

#endif /* FRAMEWORK_H */

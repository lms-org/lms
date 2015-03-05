#ifndef FRAMEWORK_H
#define FRAMEWORK_H

#include <core/datamanager.h>
#include <core/signalhandler.h>
#include <core/argumentparser.h>
#include <core/executionmanager.h>
#include <core/logger.h>
/**
 *TODO: Framework config that contains max threads for executionManager etc.
 */
namespace lms{
/**
 * @brief Main entry point into the lightweight module system.
 *
 * Create an object of this class and you will start the framwork.
 */
class Framework : public SignalHandler::Listener {
public:
    /**
     * @brief executionPath absolute path to running framework (folder that contains core and external)
     */
    static std::string programDirectory();

    /**
     * @brief Instantiate the framework, initiate datamanager and
     * execution manager and start the main loop.
     *
     * @param arguments parsed command line arguments
     */
    Framework(const ArgumentHandler& arguments);

    /**
     * @brief Destroy execution manager.
     */
    ~Framework();
private:

    RootLogger rootLogger;
    ChildLogger logger;

    ArgumentHandler argumentHandler;
    ExecutionManager executionManager;

    bool running;

    void signal(int s);
    void parseConfig();

};
}


#endif /* FRAMEWORK_H */

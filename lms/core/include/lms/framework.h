#ifndef LMS_FRAMEWORK_H
#define LMS_FRAMEWORK_H

#include <lms/datamanager.h>
#include <lms/signalhandler.h>
#include <lms/argumenthandler.h>
#include <lms/executionmanager.h>
#include <lms/logger.h>
#include "lms/clock.h"
/**
 *TODO: Framework config that contains max threads for executionManager etc.
 */
namespace lms {
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

    logging::RootLogger rootLogger;
    logging::ChildLogger logger;

    ArgumentHandler argumentHandler;
    ExecutionManager executionManager;

    bool clockEnabled;
    Clock clock;

    /**
     * @brief running just for main-while-loop if it's set to false, the programm will terminate
     */
    bool running;
    /**
     * @brief signal called by the system (Segfaults etc)
     * @param s
     */
    void signal(int s);
    /**
     * @brief parseConfig parses the framework-config
     */
    void parseConfig();

};

}  // namespace lms

#endif /* LMS_FRAMEWORK_H */

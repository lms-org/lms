#ifndef LMS_ARGUMENT_HANDLER_H
#define LMS_ARGUMENT_HANDLER_H

#include <string>
#include <vector>
#include <iostream>

#include "lms/logger.h"
#include "tclap/CmdLine.h"

namespace lms {

enum class RunLevel {
    /**
     * @brief Parse XML and LCONF files.
     */
    CONFIG = 0,

    /**
     * @brief Like CONFIG, and enable and disable all modules that should
     * be enabled according the config files.
     */
    ENABLE,

    /**
     * @brief Like ENABLE, and start module cycling.
     */
    CYCLE
};

bool runLevelByName(const std::string &str, RunLevel &runLevel);

std::ostream& operator << (std::ostream &out, RunLevel runLevel);

class ThreadsConstraint : public TCLAP::Constraint<std::string> {
public:
    std::string description() const {
        return "integer|auto";
    }

    std::string shortID() const {
        return description();
    }

    bool check(const std::string &value) const {
        return value == "auto" || isNumber(value);
    }
private:
    bool isNumber(const std::string& s) const {
        return !s.empty() && std::find_if(s.begin(),
            s.end(), [](char c) { return !std::isdigit(c); }) == s.end();
    }
};

/**
 * @brief The ArgumentHandler class used to parse the command line arguments and stores them
 */
class ArgumentHandler {
 public:
    /**
     * @brief Construct an argument handler with all
     * arguments to default.
     */
    ArgumentHandler();

    /**
     * @brief Parse the given command line arguments and store the values in the members.
     * @param argc Number of arguments
     * @param argv List of argument strings
     */
    void parseArguments(int argc, char* const*argv);

    std::string argLoadConfiguration;
    RunLevel argRunLevel;
    logging::Level argLoggingThreshold;
    bool argDefinedLoggingThreshold;
    bool argQuiet;
    std::string argLogFile;
    std::string argUser;
    std::vector<std::string> argFlags;
    std::string argProfilingFile;
    bool argMultithreaded;
    bool argThreadsAuto;
    int argThreads;
    std::string argDotFile;
};

}  // namespace lms

#endif /* LMS_ARGUMENT_HANDLER_H */

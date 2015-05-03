/**
  *Soll später ein handler werden, welcher die geparsten argumente speichert
  */
#ifndef LMS_ARGUMENT_HANDLER_H
#define LMS_ARGUMENT_HANDLER_H

#include <string>
#include <vector>
#include <iostream>

#include "lms/logger.h"

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

    /**
     * @brief Print help on a given output stream.
     * This will include information to available
     * command line arguments
     *
     * @param out the output stream where the help
     * message should be printed.
     */
    void printHelp(std::ostream *out = &std::cout) const;

    /**
     * @brief Return the "load configuration" setting. This can
     * be set by "-c arg".
     */
    std::string argLoadConfiguration() const;

    /**
     * @brief Return the "show help" setting. This can
     * be set by "-h" or "--help". The framework will
     * stop after showing help information.
     */
    bool argHelp() const;

    /**
     * @brief Return the "RunLevel" setting. This can be set by "--run-level"
     * or "-r". The framework will only run until a certain level and then exit.
     */
    RunLevel argRunLevel() const;

    /**
     * @brief All values of "--logging-prefix" as a vector of strings.
     */
    std::vector<std::string> argLoggingPrefixes() const;

    /**
     * @brief The value of the command line argument "--logging-min-level".
     */
    logging::LogLevel argLoggingMinLevel() const;

 private:
    std::string m_loadConfiguration;
    bool m_showHelp;
    RunLevel m_runLevel;
    std::vector<std::string> m_loggingPrefixes;
    logging::LogLevel m_loggingMinLevel;
};

}  // namespace lms

#endif /* LMS_ARGUMENT_HANDLER_H */

/**
  *Soll später ein handler werden, welcher die geparsten argumente speichert
  */
#ifndef LMS_ARGUMENTHANDLER_H
#define LMS_ARGUMENTHANDLER_H

#include <string>
#include <vector>
#include <core/logger.h>

namespace lms{
    /**
     * @brief The ArgumentHandler class used to parse the command line arguments and stores them
     */
    class ArgumentHandler{
    public:
        ArgumentHandler();

        /**
         * @brief Parse the given command line arguments and store the values in the members.
         * @param argc Number of arguments
         * @param argv List of argument strings
         */
        void parseArguments(int argc, char* const*argv);

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
        std::vector<std::string> m_loggingPrefixes;
        logging::LogLevel m_loggingMinLevel;
    };
}
#endif /* argumenthandler_H */

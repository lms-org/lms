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
        std::string loadConfiguration() const;

        /**
         * @brief Return the "show help" setting. This can
         * be set by "-h". The framework will stop after showing
         * help information.
         */
        bool showHelp() const;

        std::vector<std::string> loggingPrefixes() const;

        logging::LogLevel loggingMinLevel() const;

    private:
        std::string m_loadConfiguration;
        bool m_showHelp;
        std::vector<std::string> m_loggingPrefixes;
        logging::LogLevel m_loggingMinLevel;
    };
}
#endif /* argumenthandler_H */

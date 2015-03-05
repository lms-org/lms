#include <core/argumentparser.h>

#include <getopt.h>

namespace lms{
ArgumentHandler::ArgumentHandler() : m_loadConfiguration("default"), m_showHelp(false),
    m_loggingMinLevel(LogLevel::DEBUG) {
}

void ArgumentHandler::parseArguments(int argc, char* const*argv) {

    static const option LONG_OPTIONS[] = {
        {"help", no_argument, 0, 'h'},
        {"logging-min-level", required_argument, 0, 1},
        {"logging-prefix", required_argument, 0, 2}
    };

    opterr = 0;
    int c;
    while ((c = getopt_long (argc, argv, "hc:", LONG_OPTIONS, NULL)) != -1) {
        switch (c) {
            case 'c':
                m_loadConfiguration = optarg;
                break;
            case 'h': // --help
                m_showHelp = true;
                break; // Don't forget to break!
            case 1: // --logging-min-level
                m_loggingMinLevel = Logger::levelFromName(optarg);
                break;
            case 2: // --logging-prefix
                m_loggingPrefixes.push_back(optarg);
                break;
        }
    }
}

std::string ArgumentHandler::loadConfiguration() const {
    return m_loadConfiguration;
}

bool ArgumentHandler::showHelp() const {
    return m_showHelp;
}

std::vector<std::string> ArgumentHandler::loggingPrefixes() const {
    return m_loggingPrefixes;
}

LogLevel ArgumentHandler::loggingMinLevel() const {
    return m_loggingMinLevel;
}

}

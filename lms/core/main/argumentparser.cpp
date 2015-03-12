#include <getopt.h>

#include <string>
#include <vector>

#include "lms/argumentparser.h"

namespace lms {

ArgumentHandler::ArgumentHandler() : m_loadConfiguration("default"),
    m_showHelp(false), m_loggingMinLevel(logging::SMALLEST_LEVEL) {
}

void ArgumentHandler::parseArguments(int argc, char* const*argv) {
    static const option LONG_OPTIONS[] = {
        {"help", no_argument, 0, 'h'},
        {"logging-min-level", required_argument, 0, 1},
        {"logging-prefix", required_argument, 0, 2}
    };

    opterr = 0;
    int c;
    while ((c = getopt_long(argc, argv, "hc:", LONG_OPTIONS, NULL)) != -1) {
        switch (c) {
            case 'c':
                m_loadConfiguration = optarg;
                break;
            case 'h':  // --help
                m_showHelp = true;
                break;
            case 1:  // --logging-min-level
                m_loggingMinLevel = logging::Logger::levelFromName(optarg);
                break;
            case 2:  // --logging-prefix
                m_loggingPrefixes.push_back(optarg);
                break;  // Don't forget to break!
        }
    }
}

void ArgumentHandler::printHelp(std::ostream *out) const {
    *out << "LMS - Lightweight Modular System\n"
        << "Usage: lms/lms [-h] [-c config]\n"
        << "  -h, --help          Show help\n"
        << "  -c config           Load configuration (defaults to 'default')\n"
        << "  --logging-min-level Filter minimum logging level, e.g. ERROR\n"
        << "  --logging-prefix    Prefix of logging tags to filter\n"
        << std::endl;
}

std::string ArgumentHandler::argLoadConfiguration() const {
    return m_loadConfiguration;
}

bool ArgumentHandler::argHelp() const {
    return m_showHelp;
}

std::vector<std::string> ArgumentHandler::argLoggingPrefixes() const {
    return m_loggingPrefixes;
}

logging::LogLevel ArgumentHandler::argLoggingMinLevel() const {
    return m_loggingMinLevel;
}

}  // namespace lms

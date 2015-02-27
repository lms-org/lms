#include <core/argumentparser.h>

#include "getopt.h"

namespace lms{
void ArgumentHandler::parseArguments(int argc, char* const*argv) {
    m_loadConfiguration = "default";
    m_showHelp = false;

    opterr = 0;
    int c;
    while ((c = getopt (argc, argv, "hc:")) != -1) {
        switch (c) {
            case 'c':
                m_loadConfiguration = optarg;
                break;
            case 'h':
                m_showHelp = true;
                break; // Don't forget to break!
        }
    }
}

std::string ArgumentHandler::loadConfiguration() const {
    return m_loadConfiguration;
}

bool ArgumentHandler::showHelp() const {
    return m_showHelp;
}
}

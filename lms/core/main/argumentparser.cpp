#include "core/argumentparser.h"

#include <core/datamanager.h>
#include <core/shared_base.h>

#include <deque>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <core/extra/colors.h>
#include <core/configurationmanager.h>

void ArgumentHandler::parseArguments(int argc, char* const*argv) {
    m_load_configuration = "default";
    opterr = 0;
    int c;
    while ((c = getopt (argc, argv, "c:")) != -1) {
        switch (c) {
            case 'c':
                m_load_configuration = optarg;
        }
    }
}

std::string ArgumentHandler::loadConfiguration() const {
    return m_load_configuration;
}

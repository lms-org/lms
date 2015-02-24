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

ArgumentHandler::ArgumentHandler(){

}

void ArgumentHandler::parse_arguments(int argc, char* const*argv) {
    load_configuration = "default";
    opterr = 0;
    int c;
    while ((c = getopt (argc, argv, "c:")) != -1) {
        switch (c) {
            case 'c':
                load_configuration = optarg;
        }
    }
}

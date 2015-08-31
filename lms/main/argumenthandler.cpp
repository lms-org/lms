#include <string>
#include <vector>

#include "lms/argumenthandler.h"
#include "lms/extra/os.h"
#include "lms/extra/string.h"

#include "tclap/CmdLine.h"

namespace lms {

bool runLevelByName(const std::string &str, RunLevel &runLevel) {
    if (str == "CONFIG" || str == "0") {
        runLevel = RunLevel::CONFIG;
        return true;
    } else if (str == "ENABLE" || str == "1") {
        runLevel = RunLevel::ENABLE;
        return true;
    } else if (str == "CYCLE" || str == "2") {
        runLevel = RunLevel::CYCLE;
        return true;
    }

    return false;
}

std::ostream& operator << (std::ostream &out, RunLevel runLevel) {
    switch(runLevel) {
    case RunLevel::CONFIG:
        out << "CONFIG";
        break;
    case RunLevel::ENABLE:
        out << "ENABLE";
        break;
    case RunLevel::CYCLE:
        out << "CYCLE";
        break;
    }
    return out;
}

ArgumentHandler::ArgumentHandler() : argLoadConfiguration(""),
    argRunLevel(RunLevel::CYCLE),
    argLoggingMinLevel(logging::LogLevel::ALL), argQuiet(false), argUser(""),
    argProfiling(false), argConfigMonitor(false), argMultithreaded(false),
    argThreadsAuto(false), argThreads(1) {

    argUser = lms::extra::username();
}

void ArgumentHandler::parseArguments(int argc, char* const*argv) {
#ifdef _WIN32
#define USER_ENV "$USERNAME on Windows"
#elif __APPLE__
#define USER_ENV "$USER on OSX"
#else
#define USER_ENV "$LOGNAME on Unix"
#endif
    std::vector<std::string> debugLevels = {"DEBUG", "INFO", "WARN", "ERROR"};
    TCLAP::ValuesConstraint<std::string> debugConstraint(debugLevels);

    std::vector<std::string> runLevels = {"0", "1", "2", "CONFIG", "ENABLE",
                                          "CYCLE"};
    TCLAP::ValuesConstraint<std::string> runLevelsConstraint(runLevels);

    ThreadsConstraint threadsConstraint;

    TCLAP::CmdLine cmd("LMS - Lightweight Modular System", ' ', "1.0");
    TCLAP::ValueArg<std::string> runLevelArg("r", "run-level",
        "Execute until a certain run level",
        false, "CYCLE", &runLevelsConstraint, cmd);
    TCLAP::ValueArg<std::string> loggingMinLevelArg("", "logging-min-level",
        "Filter logging by minimum logging level",
        false, "DEBUG", &debugConstraint, cmd);
    TCLAP::MultiArg<std::string> loggingPrefixArg("", "logging-prefix",
        "Filter logging by prefix of logging tags",
        false, "string", cmd);
    TCLAP::ValueArg<std::string> userArg("", "user",
        "Set the username, used for config loading, defaults to " USER_ENV,
        false, lms::extra::username(), "string", cmd);
    TCLAP::ValueArg<std::string> configArg("c", "config",
        "Load XML configuration file",
        false, "framework_conf", "string", cmd);
    TCLAP::ValueArg<std::string> logFileArg("", "log-file",
        "Log to the given file",
        false, "", "path", cmd);
    TCLAP::SwitchArg quietSwitch("q", "quiet",
        "Do not log anything to stdout",
        cmd, false);
    TCLAP::ValueArg<std::string> flagsArg("", "flags",
        "Config flags, can be used in <if> tags",
        false, "", "string-list", cmd);
    TCLAP::SwitchArg profilingSwitch("", "profiling",
        "Measure execution time of all modules",
        cmd, false);
    TCLAP::SwitchArg configMonitorSwitch("", "config-monitor",
        "Enable live config monitoring",
        cmd, false);
    TCLAP::ValueArg<std::string> threadsArg("", "threads",
        "Enable multithreading, number of threads or auto",
        false, "", &threadsConstraint, cmd);

    cmd.parse(argc, argv);

    argLoadConfiguration = configArg.getValue();
    logging::levelFromName(loggingMinLevelArg.getValue(), argLoggingMinLevel);
    argLoggingPrefixes = loggingPrefixArg.getValue();
    argUser = userArg.getValue();
    argLogFile = logFileArg.getValue();
    argQuiet = quietSwitch.getValue();
    argFlags = lms::extra::split(flagsArg.getValue(), ',');
    argProfiling = profilingSwitch.getValue();
    argConfigMonitor = configMonitorSwitch.getValue();
    runLevelByName(runLevelArg.getValue(), argRunLevel);
    if(threadsArg.isSet()) {
        argMultithreaded = true;
        if(threadsArg.getValue() == std::string("auto")) {
            argThreadsAuto = true;
        } else {
            argThreads = atoi(threadsArg.getValue().c_str());
        }
    }
#undef USER_ENV
}

}  // namespace lms

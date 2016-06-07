#include <string>
#include <vector>
#include <algorithm>

#include "lms/internal/argumenthandler.h"
#include "lms/internal/os.h"
#include "lms/internal/string.h"
#include "lms/definitions.h"

#include "tclap/CmdLine.h"

namespace lms {
namespace internal {

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

std::ostream &operator<<(std::ostream &out, RunLevel runLevel) {
    switch (runLevel) {
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

ArgumentHandler::ArgumentHandler()
    : argLoadConfiguration(""), argRunLevel(RunLevel::CYCLE),
      argLoggingThreshold(logging::Level::ALL),
      argDefinedLoggingThreshold(false), argQuiet(false), argUser(""),
      argMultithreaded(false), argThreadsAuto(false), argThreads(1),
      argDebug(false), argEnableLoad(false), argEnableSave(false) {

    argUser = username();
}

std::string ArgumentHandler::slug(std::string const &tag) {
    std::string res(tag);
    std::replace(res.begin(), res.end(), ' ', '-');
    std::replace(res.begin(), res.end(), '/', '-');
    std::replace(res.begin(), res.end(), '\\', '-');
    return res;
}

void ArgumentHandler::parseArguments(int argc, char *const *argv) {
#ifdef _WIN32
#define USER_ENV "$USERNAME on Windows"
#elif __APPLE__
#define USER_ENV "$USER on OSX"
#else
#define USER_ENV "$LOGNAME on Unix"
#endif
    std::vector<std::string> debugLevels = {"ALL",  "DEBUG", "INFO",
                                            "WARN", "ERROR", "OFF"};
    TCLAP::ValuesConstraint<std::string> debugConstraint(debugLevels);

    std::vector<std::string> runLevels = {"0",      "1",      "2",
                                          "CONFIG", "ENABLE", "CYCLE"};
    TCLAP::ValuesConstraint<std::string> runLevelsConstraint(runLevels);

    ThreadsConstraint threadsConstraint;

    TCLAP::CmdLine cmd("LMS - Lightweight Modular System", ' ',
                       LMS_VERSION_STRING);
    TCLAP::ValueArg<std::string> runLevelArg(
        "r", "run-level", "Execute until a certain run level", false, "CYCLE",
        &runLevelsConstraint, cmd);
    TCLAP::ValueArg<std::string> loggingMinLevelArg(
        "", "logging-threshold", "Filter logging by minimum logging level",
        false, "ALL", &debugConstraint, cmd);
    TCLAP::ValueArg<std::string> userArg(
        "", "user",
        "Set the username, used for config loading, defaults to " USER_ENV,
        false, username(), "string", cmd);
    TCLAP::ValueArg<std::string> configArg("c", "config",
                                           "Load XML configuration file", false,
                                           "framework_conf", "string", cmd);
    TCLAP::ValueArg<std::string> logFileArg(
        "", "log-file", "Log to the given file", false, "", "path", cmd);
    TCLAP::SwitchArg quietSwitch("q", "quiet", "Do not log anything to stdout",
                                 cmd, false);
    TCLAP::ValueArg<std::string> flagsArg(
        "", "flags", "Config flags, can be used in <if> tags", false, "",
        "string-list", cmd);
    TCLAP::ValueArg<std::string> profilingArg(
        "", "profiling",
        "Measure execution time of all modules and dump to a file", false, "",
        "path", cmd);
    TCLAP::ValueArg<std::string> threadsArg(
        "", "threads", "Enable multithreading, number of threads or auto",
        false, "", &threadsConstraint, cmd);
    TCLAP::SwitchArg dagSwitch(
        "", "dag", "Dump the dependency graph as a dot file", cmd, false);
    TCLAP::SwitchArg debugSwitch(
        "", "debug", "Make a ridiculous number of debug outputs", cmd, false);
    TCLAP::ValueArg<std::string> debugServerArg(
        "", "debug-server",
        "Start a local debug server providing logging and profiling data",
        false, "", "path|port", cmd);
    TCLAP::ValueArg<std::string> enableLoadArg(
        "", "enable-load",
        "Enable all loading modules and set a default load path", false, "",
        "path", cmd);
    TCLAP::SwitchArg enableSaveSwitch("", "enable-save",
                                      "Enable all saving modules", cmd, false);
    TCLAP::ValueArg<std::string> enableSaveTag(
        "", "enable-save-tag",
        "Enable all saving modules and mark the save folder", false, "", "tag",
        cmd);
    TCLAP::ValueArg<std::string> configPathArg("", "config-path",
                                               "Set configs path explicitly",
                                               false, "", "path", cmd);

    cmd.parse(argc, argv);

    argLoadConfiguration = configArg.getValue();
    logging::levelFromName(loggingMinLevelArg.getValue(), argLoggingThreshold);
    argDefinedLoggingThreshold = loggingMinLevelArg.isSet();
    argUser = userArg.getValue();
    argLogFile = logFileArg.getValue();
    argQuiet = quietSwitch.getValue();
    argFlags = split(flagsArg.getValue(), ',');
    argProfilingFile = profilingArg.getValue();
    argDebug = debugSwitch.getValue();
    runLevelByName(runLevelArg.getValue(), argRunLevel);
    if (threadsArg.isSet()) {
        argMultithreaded = true;
        if (threadsArg.getValue() == std::string("auto")) {
            argThreadsAuto = true;
        } else {
            argThreads = atoi(threadsArg.getValue().c_str());
        }
    }
    argDAG = dagSwitch.getValue();
    argEnableLoad = enableLoadArg.isSet();
    argEnableLoadPath = enableLoadArg.getValue();
    argEnableSave = enableSaveSwitch.getValue() || enableSaveTag.isSet();
    argEnableDebugServer = debugServerArg.isSet();
    argDebugServerBind = debugServerArg.getValue();

    argEnableSaveTag = slug(enableSaveTag.getValue());

    if (argEnableLoad) {
        argFlags.push_back("__load");
    }
    if (argEnableSave) {
        argFlags.push_back("__save");
    }

    configPath = configPathArg.getValue();
#undef USER_ENV
}

} // namespace internal
} // namespace lms

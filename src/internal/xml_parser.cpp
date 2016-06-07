#include <cstring>

#include "lms/internal/xml_parser.h"
#include "lms/internal/os.h"
#include "lms/internal/string.h"
#include "lms/internal/module_wrapper.h"

namespace lms {
namespace internal {

bool evaluateSet(const std::string &condition,
                 const std::vector<std::string> &flags) {
    return std::find(flags.begin(), flags.end(), condition) != flags.end();
}

bool evaluateNotSet(const std::string &condition,
                    const std::vector<std::string> &flags) {
    return std::find(flags.begin(), flags.end(), condition) == flags.end();
}

bool evaluateAnyOf(const std::vector<std::string> &condition,
                   const std::vector<std::string> &flags) {
    return std::find_first_of(condition.begin(), condition.end(), flags.begin(),
                              flags.end()) != condition.end();
}

bool evaluateAllOf(const std::vector<std::string> &condition,
                   const std::vector<std::string> &flags) {
    for (const std::string &cond : condition) {
        if (std::find(flags.begin(), flags.end(), cond) == flags.end()) {
            return false;
        }
    }

    return true;
}

bool evaluateNothingOf(const std::vector<std::string> &condition,
                       const std::vector<std::string> &flags) {
    for (const std::string &cond : condition) {
        if (std::find(flags.begin(), flags.end(), cond) != flags.end()) {
            return false;
        }
    }

    return true;
}

void preprocessXML(pugi::xml_node node, const std::vector<std::string> &flags) {
    if (std::string("/framework/module/config") == node.path()) {
        // skip <config> nodes
        return;
    }

    for (pugi::xml_node child = node.child("if"); child;) {
        if (std::string("if") == child.name()) {
            bool result = false;

            pugi::xml_attribute setAttr = child.attribute("set");
            pugi::xml_attribute notSetAttr = child.attribute("notSet");
            pugi::xml_attribute anyOfAttr = child.attribute("anyOf");
            pugi::xml_attribute allOfAttr = child.attribute("allOf");
            pugi::xml_attribute nothingOfAttr = child.attribute("nothingOf");

            if (setAttr) {
                result = evaluateSet(setAttr.value(), flags);
            } else if (notSetAttr) {
                result = evaluateNotSet(notSetAttr.value(), flags);
            } else if (anyOfAttr) {
                result = evaluateAnyOf(split(anyOfAttr.value(), ','), flags);
            } else if (allOfAttr) {
                result = evaluateAllOf(split(allOfAttr.value(), ','), flags);
            } else if (nothingOfAttr) {
                result =
                    evaluateNothingOf(split(nothingOfAttr.value(), ','), flags);
            } else {
                std::cout << "Failed to preprocess XML <if>" << std::endl;
            }

            // if the condition evaluated to true
            if (result) {
                // then move all children of <if> to be siblings of <if>

                pugi::xml_node moveNode;
                while ((moveNode = child.first_child())) {
                    node.insert_move_after(moveNode, child);
                }

                node.remove_child(child);
            } else {
                node.remove_child(child);
            }

            // reset child
            child = node.first_child();
        } else {
            // go further
            child = child.next_sibling("if");
        }
    }

    for (pugi::xml_node child : node) {
        preprocessXML(child, flags);
    }
}

XmlParser::XmlParser(Framework &framework, Runtime *runtime,
                     const ArgumentHandler &args)
    : m_framework(framework), m_runtime(runtime), m_args(args) {}

void XmlParser::parseConfig(XmlParser::LoadConfigFlag flag,
                            const std::string &argLoadConfig,
                            std::string const &configPath) {
    std::string path = configPath;
    if (argLoadConfig.empty()) {
        path += "/framework_conf.xml";
    } else {
        path += "/" + argLoadConfig + ".xml";
    }

    if (fileType(path) == FileType::REGULAR_FILE) {
        parseFile(path, flag);
        return;
    } else {
        errorFile(path);
    }
}

void parseModuleConfig(pugi::xml_node node, Config &config,
                       const std::string &key) {
    for (auto subnode : node.children()) {
        if (subnode.type() == pugi::node_element) {
            std::string newKey = subnode.attribute("name").as_string();
            if (!key.empty()) {
                newKey = key + "." + newKey;
            }

            if (std::strcmp("group", subnode.name()) == 0) {
                parseModuleConfig(subnode, config, newKey);
            } else {
                config.set<std::string>(newKey, trim(subnode.child_value()));
            }
        }
    }
}

void XmlParser::parseModulesToEnable(pugi::xml_node node) {
    ExecutionManager::EnableConfig &config =
        m_runtime->executionManager().config();

    lms::logging::Level defaultModuleLevel = lms::logging::Level::ALL;

    // get attribute "logLevel" of node <modulesToLoad>
    // its value will be the default for logLevel of <module>
    pugi::xml_attribute globalLogLevelAttr = node.attribute("logLevel");
    if (globalLogLevelAttr) {
        lms::logging::levelFromName(globalLogLevelAttr.value(),
                                    defaultModuleLevel);
    }

    for (pugi::xml_node moduleNode : node.children("module")) {
        // parse module content
        std::pair<std::string, logging::Level> mod;
        mod.first = moduleNode.child_value();

        // get the attribute "logLevel"
        pugi::xml_attribute logLevelAttr = moduleNode.attribute("logLevel");
        if (logLevelAttr) {
            lms::logging::levelFromName(logLevelAttr.value(), mod.second);
        } else {
            mod.second = defaultModuleLevel;
        }

        config.push_back(mod);
    }
}

logging::ThresholdFilter *XmlParser::parseLogging(pugi::xml_node node) {
    logging::Level defaultThreshold = logging::Level::ALL;

    pugi::xml_attribute levelAttr = node.attribute("logLevel");
    logging::levelFromName(levelAttr.as_string("ALL"), defaultThreshold);

    logging::ThresholdFilter *filter =
        new logging::ThresholdFilter(defaultThreshold);

    for (pugi::xml_node filterNode : node.children("filter")) {
        pugi::xml_attribute tagPrefixAttr = filterNode.attribute("tagPrefix");
        pugi::xml_attribute logLevelAttr = filterNode.attribute("logLevel");

        if (!tagPrefixAttr) {
            errorMissingAttr(filterNode, tagPrefixAttr);
        }

        if (!logLevelAttr) {
            errorMissingAttr(filterNode, logLevelAttr);
        }

        logging::Level level;
        logging::levelFromName(logLevelAttr.as_string(), level);
        filter->addPrefix(tagPrefixAttr.as_string(), level);
    }

    return filter;
}

void XmlParser::parseExecution(pugi::xml_node node, Runtime *runtime) {
    pugi::xml_node clockNode = node.child("clock");
    pugi::xml_node mainThreadNode = node.child("mainThread");
    pugi::xml_node pausedNode = node.child("paused");

    Clock &clock = runtime->clock();

    if (clockNode) {
        std::string clockUnit;
        std::int64_t clockValue = 0;

        pugi::xml_attribute sleepAttr = clockNode.attribute("sleep");
        pugi::xml_attribute compensateAttr = clockNode.attribute("compensate");
        pugi::xml_attribute unitAttr = clockNode.attribute("unit");
        pugi::xml_attribute valueAttr = clockNode.attribute("value");
        pugi::xml_attribute watchDog = clockNode.attribute("watchDog");

        clock.enabledSlowWarning(true);

        if (sleepAttr) {
            clock.enabledSleep(sleepAttr.as_bool());
        } else {
            // if not enabled attribute is given then the clock is considered
            // to be disabled
            clock.enabledSleep(false);
        }

        if (valueAttr) {
            clockValue = valueAttr.as_llong();
        } else {
            clock.enabledSlowWarning(false);
            errorMissingAttr(clockNode, valueAttr);
        }

        if (unitAttr) {
            clockUnit = unitAttr.value();
        } else {
            clock.enabledSlowWarning(false);
            errorMissingAttr(clockNode, unitAttr);
        }

        if (compensateAttr) {
            clock.enabledCompensate(compensateAttr.as_bool());
        } else {
            clock.enabledCompensate(false);
        }

        if (clockUnit == "hz") {
            clock.cycleTime(Time::fromMicros(1000000 / clockValue));
        } else if (clockUnit == "ms") {
            clock.cycleTime(Time::fromMillis(clockValue));
        } else if (clockUnit == "us") {
            clock.cycleTime(Time::fromMicros(clockValue));
        } else {
            clock.enabledSlowWarning(false);
            errorInvalidAttr(clockNode, unitAttr, "ms/us/hz");
        }

        if (watchDog) {
            WatchDog &dog = runtime->executionManager().dog();

            if (clockUnit == "hz") {
                dog.watch(Time::fromMicros(1000000 / watchDog.as_llong()));
            } else if (clockUnit == "ms") {
                dog.watch(Time::fromMillis(watchDog.as_llong()));
            } else if (clockUnit == "us") {
                dog.watch(Time::fromMicros(watchDog.as_llong()));
            } else {
                errorInvalidAttr(clockNode, watchDog, "ms/us/hz");
            }
        }
    }

    if (mainThreadNode) {
        runtime->executionType(ExecutionType::ONLY_MAIN_THREAD);
    }

    if (pausedNode) {
        runtime->pause();
    }
}

void XmlParser::parseInclude(pugi::xml_node node,
                             const std::string &currentFile,
                             LoadConfigFlag flag) {
    pugi::xml_attribute srcAttr = node.attribute("src");

    if (srcAttr) {
        std::string includePath = srcAttr.value();
        if (isAbsolute(includePath)) {
            // if absolute then start from configs dir
            includePath = LMS_CONFIGS + includePath;
        } else {
            // otherwise go from current file's directory
            includePath = dirname(currentFile) + "/" + includePath;
        }
        parseFile(includePath, flag);
    } else {
        errorMissingAttr(node, srcAttr);
    }
}

void XmlParser::parseRuntime(pugi::xml_node node,
                             const std::string &currentFile,
                             LoadConfigFlag flag) {
    pugi::xml_attribute srcAttr = node.attribute("src");
    pugi::xml_attribute nameAttr = node.attribute("name");

    if (srcAttr && nameAttr) {
        std::string includePath = srcAttr.value();
        if (isAbsolute(includePath)) {
            // if absolute then start from configs dir
            includePath = LMS_CONFIGS + includePath;
        } else {
            // otherwise go from current file's directory
            includePath = dirname(currentFile) + "/" + includePath;
        }

        Runtime *temp = m_runtime;
        if (flag == LoadConfigFlag::LOAD_EVERYTHING) {
            m_runtime = new Runtime(nameAttr.value(), m_framework);
            m_framework.registerRuntime(m_runtime);
        } else if (m_framework.hasRuntime(nameAttr.value())) {
            // reload configs && runtime is installed
            m_runtime = m_framework.getRuntimeByName(nameAttr.value());
        } else {
            std::cout << "Runtime is not available: " << nameAttr.value()
                      << std::endl;
        }
        parseFile(includePath, flag);
        m_runtime = temp;
    } else if (!srcAttr) {
        errorMissingAttr(node, srcAttr);
    } else if (!nameAttr) {
        errorMissingAttr(node, nameAttr);
    }
}

void XmlParser::parseModules(pugi::xml_node node,
                             const std::string &currentFile,
                             LoadConfigFlag flag) {
    std::shared_ptr<ModuleWrapper> module =
        std::make_shared<ModuleWrapper>(m_runtime);

    pugi::xml_attribute nameAttr = node.attribute("name");
    pugi::xml_attribute libAttr = node.attribute("lib");
    pugi::xml_attribute classAttr = node.attribute("class");
    pugi::xml_attribute mainThreadAttr = node.attribute("mainThread");

    if (nameAttr) {
        module->name(nameAttr.as_string());
    } else {
        errorMissingAttr(node, nameAttr);
    }

    if (libAttr) {
        module->lib(libAttr.as_string());
    } else {
        errorMissingAttr(node, libAttr);
    }

    if (classAttr) {
        module->clazz(classAttr.as_string());
    } else {
        errorMissingAttr(node, classAttr);
    }

    if (mainThreadAttr.as_bool()) {
        module->executionType = ExecutionType::ONLY_MAIN_THREAD;
    } else {
        module->executionType = ExecutionType::NEVER_MAIN_THREAD;
    }

    // parse all channel mappings
    // TODO This now deprecated in favor for channelHint
    for (pugi::xml_node mappingNode : node.children("channelMapping")) {
        pugi::xml_attribute fromAttr = mappingNode.attribute("from");
        pugi::xml_attribute toAttr = mappingNode.attribute("to");
        pugi::xml_attribute priorityAttr = mappingNode.attribute("priority");

        if (!fromAttr) {
            errorMissingAttr(mappingNode, fromAttr);
        }

        if (!toAttr) {
            errorMissingAttr(mappingNode, toAttr);
        }

        if (fromAttr && toAttr) {
            int priority = priorityAttr ? priorityAttr.as_int() : 0;
            module->channelMapping[fromAttr.value()] =
                std::make_pair(toAttr.value(), priority);
        }
    }

    for (pugi::xml_node channelNode : node.children("channelHint")) {
        pugi::xml_attribute nameAttr = channelNode.attribute("name");
        pugi::xml_attribute mapToAttr = channelNode.attribute("mapTo");
        pugi::xml_attribute priorityAttr = channelNode.attribute("priority");

        if (!nameAttr) {
            errorMissingAttr(channelNode, nameAttr);
            continue;
        }

        std::string mapTo = mapToAttr ? mapToAttr.value() : nameAttr.value();
        int priority = priorityAttr ? priorityAttr.as_int() : 0;

        module->channelMapping[nameAttr.value()] =
            std::make_pair(mapTo, priority);
    }

    // parse all config
    for (pugi::xml_node configNode : node.children("config")) {
        pugi::xml_attribute srcAttr = configNode.attribute("src");
        pugi::xml_attribute nameAttr = configNode.attribute("name");
        pugi::xml_attribute userAttr = configNode.attribute("user");

        if (userAttr) {
            std::vector<std::string> allowedUsers =
                split(std::string(userAttr.value()), ',');

            if (std::find(allowedUsers.begin(), allowedUsers.end(),
                          m_args.argUser) == allowedUsers.end()) {
                continue;
            }
        }

        std::string name = "default";
        if (nameAttr) {
            name = nameAttr.value();
        }

        if (srcAttr) {
            std::string lconfPath = srcAttr.value();

            if (isAbsolute(lconfPath)) {
                lconfPath = LMS_CONFIGS + lconfPath;
            } else {
                lconfPath = dirname(currentFile) + "/" + lconfPath;
            }

            bool loadResult = module->configs[name].loadFromFile(lconfPath);
            if (!loadResult) {
                errorFile(lconfPath);
            } else {
                m_files.push_back(lconfPath);
            }
        } else {
            // if there was no src attribut then parse the tag's content
            parseModuleConfig(configNode, module->configs[name], "");
        }
    }

    if (flag != LoadConfigFlag::ONLY_MODULE_CONFIG) {
        m_runtime->executionManager().installModule(module);
    } else {
        m_runtime->executionManager().bufferModule(module);
    }
}

void XmlParser::parseService(pugi::xml_node node,
                             const std::string &currentFile,
                             LoadConfigFlag flag) {
    std::shared_ptr<ServiceWrapper> service =
        std::make_shared<ServiceWrapper>(&m_framework);

    pugi::xml_attribute nameAttr = node.attribute("name");
    pugi::xml_attribute libAttr = node.attribute("lib");
    pugi::xml_attribute classAttr = node.attribute("class");
    pugi::xml_attribute logLevelAttr = node.attribute("logLevel");

    if (nameAttr) {
        service->name(nameAttr.as_string());
    } else {
        errorMissingAttr(node, nameAttr);
    }

    if (libAttr) {
        service->lib(libAttr.as_string());
    } else {
        errorMissingAttr(node, libAttr);
    }

    if (classAttr) {
        service->clazz(classAttr.as_string());
    } else {
        errorMissingAttr(node, classAttr);
    }

    logging::Level defaultLevel = logging::Level::ALL;
    if (logLevelAttr) {
        logging::levelFromName(logLevelAttr.as_string(), defaultLevel);
    }
    service->defaultLogLevel(defaultLevel);

    // parse all config
    for (pugi::xml_node configNode : node.children("config")) {
        pugi::xml_attribute srcAttr = configNode.attribute("src");
        pugi::xml_attribute nameAttr = configNode.attribute("name");
        pugi::xml_attribute userAttr = configNode.attribute("user");

        if (userAttr) {
            std::vector<std::string> allowedUsers =
                split(std::string(userAttr.value()), ',');

            if (std::find(allowedUsers.begin(), allowedUsers.end(),
                          m_args.argUser) == allowedUsers.end()) {
                continue;
            }
        }

        std::string name = "default";
        if (nameAttr) {
            name = nameAttr.value();
        }

        if (srcAttr) {
            std::string lconfPath = srcAttr.value();

            if (isAbsolute(lconfPath)) {
                lconfPath = LMS_CONFIGS + lconfPath;
            } else {
                lconfPath = dirname(currentFile) + "/" + lconfPath;
            }

            bool loadResult = service->getConfig(name).loadFromFile(lconfPath);
            if (!loadResult) {
                errorFile(lconfPath);
            } else {
                m_files.push_back(lconfPath);
            }
        } else {
            // if there was no src attribut then parse the tag's content
            parseModuleConfig(configNode, service->getConfig(name), "");
        }
    }

    if (flag != LoadConfigFlag::ONLY_MODULE_CONFIG) {
        m_runtime->framework().installService(service);
    } else {
        m_runtime->framework().reloadService(service);
    }
}

void XmlParser::parseFile(const std::string &file, LoadConfigFlag flag) {
    std::ifstream ifs(file);

    if (!ifs.is_open()) {
        errorFile(file);
        return;
    }

    m_files.push_back(file);

    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load(ifs);
    if (!result) {
        errorPugiParseResult(file, result);
        return;
    }

    pugi::xml_node rootNode = doc.child("framework");
    preprocessXML(rootNode, m_args.argFlags);

    for (pugi::xml_node node : rootNode.children()) {
        if (std::string("execution") == node.name()) {
            parseExecution(node, m_runtime);
        } else if (std::string("logging") == node.name()) {
            m_filter.reset(parseLogging(node));
        } else if (std::string("modulesToEnable") == node.name()) {
            parseModulesToEnable(node);
        } else if (std::string("module") == node.name()) {
            parseModules(node, file, flag);
        } else if (std::string("include") == node.name()) {
            parseInclude(node, file, flag);
        } else if (std::string("runtime") == node.name()) {
            parseRuntime(node, file, flag);
        } else if (std::string("service") == node.name()) {
            parseService(node, file, flag);
        } else {
            errorUnknownNode(node);
        }
    }
}

std::vector<std::string> const &XmlParser::files() const { return m_files; }

std::vector<std::string> const &XmlParser::errors() const { return m_errors; }

void XmlParser::errorMissingAttr(pugi::xml_node node,
                                 pugi::xml_attribute attr) {
    m_errors.push_back(std::string("Missing attribute ") + attr.name() +
                       " in node " + node.path());
}

void XmlParser::errorInvalidAttr(pugi::xml_node node, pugi::xml_attribute attr,
                                 const std::string &expectedValue) {
    m_errors.push_back(std::string("Invalid value for attribute ") +
                       attr.name() + " in node " + node.path() +
                       ", Value is \"" + attr.value() + "\" but expected \"" +
                       expectedValue + "\"");
}

void XmlParser::errorInvalidNodeContent(pugi::xml_node node,
                                        const std::string &expected) {
    m_errors.push_back("Invalid text content in node " + node.path() +
                       ", content: \"" + node.child_value() +
                       "\", expected \"" + expected + "\"");
}

void XmlParser::errorFile(const std::string &file) {
    m_errors.push_back("Could not open file " + file);
}

void XmlParser::errorPugiParseResult(const std::string &file,
                                     const pugi::xml_parse_result &result) {
    m_errors.push_back(std::string() + "Failed to parse " + file + " as XML: " +
                       std::to_string(result.offset) + " " +
                       result.description());
}

void XmlParser::errorUnknownNode(pugi::xml_node node) {
    m_errors.push_back("Unknown node " + node.path());
}

std::unique_ptr<logging::ThresholdFilter> XmlParser::filter() {
    return std::move(m_filter);
}

} // namespace internal
} // namespace lms
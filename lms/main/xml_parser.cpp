#include "lms/xml_parser.h"
#include "lms/extra/string.h"
#include "lms/clock.h"
#include "lms/framework.h"
#include "lms/definitions.h"

#include <algorithm>
#include <iostream>
#include <utility>

namespace lms {

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
    return std::find_first_of(condition.begin(), condition.end(),
                       flags.begin(), flags.end()) != condition.end();
}

bool evaluateAllOf(const std::vector<std::string> &condition,
                   const std::vector<std::string> &flags) {
    for(const std::string &cond : condition) {
        if(std::find(flags.begin(), flags.end(), cond) == flags.end()) {
            return false;
        }
    }

    return true;
}

bool evaluateNothingOf(const std::vector<std::string> &condition,
                   const std::vector<std::string> &flags) {
    for(const std::string &cond : condition) {
        if(std::find(flags.begin(), flags.end(), cond) != flags.end()) {
            return false;
        }
    }

    return true;
}

void preprocessXML(pugi::xml_node node, const std::vector<std::string> &flags) {
    if(std::string("/framework/module/config") == node.path()) {
        // skip <config> nodes
        return;
    }

    for(pugi::xml_node child = node.child("if"); child; ) {
        if(std::string("if") == child.name()) {
            bool result = false;

            pugi::xml_attribute setAttr = child.attribute("set");
            pugi::xml_attribute notSetAttr = child.attribute("notSet");
            pugi::xml_attribute anyOfAttr = child.attribute("anyOf");
            pugi::xml_attribute allOfAttr = child.attribute("allOf");
            pugi::xml_attribute nothingOfAttr = child.attribute("nothingOf");

            if(setAttr) {
                result = evaluateSet(setAttr.value(), flags);
            } else if(notSetAttr) {
                result = evaluateNotSet(notSetAttr.value(), flags);
            } else if(anyOfAttr) {
                result = evaluateAnyOf(lms::extra::split(anyOfAttr.value(), ','), flags);
            } else if(allOfAttr) {
                result = evaluateAllOf(lms::extra::split(allOfAttr.value(), ','), flags);
            } else if(nothingOfAttr) {
                result = evaluateNothingOf(lms::extra::split(nothingOfAttr.value(), ','), flags);
            } else {
                std::cout << "Failed to preprocess XML <if>" << std::endl;
            }

            // if the condition evaluated to true
            if(result) {
                // then move all children of <if> to be siblings of <if>

                pugi::xml_node moveNode;
                while((moveNode = child.first_child())) {
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

    for(pugi::xml_node child : node) {
        preprocessXML(child, flags);
    }
}

XmlParser::XmlParser(Framework &framework, Runtime* runtime, const ArgumentHandler &args) :
    m_framework(framework), m_runtime(runtime), m_args(args) {}

void XmlParser::parseConfig(XmlParser::LoadConfigFlag flag, const std::string &argLoadConfig){

    std::string configPath = std::string(LMS_CONFIGS "/");
    if(argLoadConfig.empty()) {
        configPath += "framework_conf.xml";
    } else {
        configPath += argLoadConfig + ".xml";
    }

    parseFile(configPath, flag);
}

void parseModuleConfig(pugi::xml_node node, ModuleConfig &config,
                       const std::string &key) {
    // if node has no children
    if(node.type() == pugi::node_pcdata) {
        config.set<std::string>(key, extra::trim(node.value()));
    } else if(node.type() == pugi::node_element) {
        std::string newKey;

        for(pugi::xml_node subnode : node.children()) {
            if(key.empty()) {
                newKey = subnode.name();
            } else if(subnode.type() == pugi::node_element) {
                newKey = key + "." + subnode.name();
            } else {
                newKey = key;
            }
            parseModuleConfig(subnode, config, newKey);
        }
    }
}

void XmlParser::parseModulesToEnable(pugi::xml_node node) {
    std::string name = "default";

    pugi::xml_attribute nameAttr = node.attribute("name");
    if(nameAttr) {
        name = nameAttr.as_string();
    }

    ExecutionManager::EnableConfig &config = m_runtime->executionManager().config(name);

    lms::logging::Level defaultModuleLevel = lms::logging::Level::ALL;

    // get attribute "logLevel" of node <modulesToLoad>
    // its value will be the default for logLevel of <module>
    pugi::xml_attribute globalLogLevelAttr = node.attribute("logLevel");
    if(globalLogLevelAttr) {
        lms::logging::levelFromName(globalLogLevelAttr.value(), defaultModuleLevel);
    }

    for (pugi::xml_node moduleNode : node.children("module")){
        //parse module content
        std::pair<std::string, logging::Level> mod;
        mod.first = moduleNode.child_value();

        // get the attribute "logLevel"
        pugi::xml_attribute logLevelAttr = moduleNode.attribute("logLevel");
        if(logLevelAttr) {
            lms::logging::levelFromName(logLevelAttr.value(), mod.second);
        } else {
            mod.second = defaultModuleLevel;
        }

        config.push_back(mod);
    }
}

logging::ThresholdFilter* XmlParser::parseLogging(pugi::xml_node node) {
    logging::Level defaultThreshold = logging::Level::ALL;

    pugi::xml_attribute levelAttr = node.attribute("logLevel");
    logging::levelFromName(levelAttr.as_string("ALL"), defaultThreshold);

    logging::ThresholdFilter *filter = new logging::ThresholdFilter(defaultThreshold);

    for(pugi::xml_node filterNode : node.children("filter")) {
        pugi::xml_attribute tagPrefixAttr = filterNode.attribute("tagPrefix");
        pugi::xml_attribute logLevelAttr = filterNode.attribute("logLevel");

        if(! tagPrefixAttr) {
            errorMissingAttr(filterNode, tagPrefixAttr);
        }

        if(! logLevelAttr) {
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
    pugi::xml_node executionTypeNode = node.child("executionType");
    pugi::xml_node pausedNode = node.child("paused");

    Clock& clock = runtime->clock();

    if(clockNode) {
        std::string clockUnit;
        std::int64_t clockValue = 0;

        pugi::xml_attribute enabledAttr = clockNode.attribute("enabled");
        pugi::xml_attribute unitAttr = clockNode.attribute("unit");
        pugi::xml_attribute valueAttr = clockNode.attribute("value");

        if(enabledAttr) {
            clock.enabled(enabledAttr.as_bool());
        } else {
            // if not enabled attribute is given then the clock is considered
            // to be disabled
            clock.enabled(false);
        }

        if(valueAttr) {
            clockValue = valueAttr.as_llong();
        } else {
            clock.enabled(false);
            errorMissingAttr(clockNode, valueAttr);
        }

        if(unitAttr) {
            clockUnit = unitAttr.value();
        } else {
            runtime->clock().enabled(false);
            errorMissingAttr(clockNode, unitAttr);
        }

        if(clock.enabled()) {
            if(clockUnit == "hz") {
                clock.cycleTime(extra::PrecisionTime::fromMicros(1000000 / clockValue));
            } else if(clockUnit == "ms") {
                clock.cycleTime(extra::PrecisionTime::fromMillis(clockValue));
            } else if(clockUnit == "us") {
                clock.cycleTime(extra::PrecisionTime::fromMicros(clockValue));
            } else {
                clock.enabled(false);
                errorInvalidAttr(clockNode, unitAttr, "ms/us/hz");
            }
        }
    }

    ExecutionType type = ExecutionType::NEVER_MAIN_THREAD;

    if(executionTypeNode) {
        std::string executionType = executionTypeNode.child_value();

        if(! lms::executionTypeByName(executionType, type)) {
            errorInvalidNodeContent(executionTypeNode, "ONLY_MAIN_THREAD|NEVER_MAIN_THREAD");
        }
    }

    if(pausedNode) {
        runtime->pause();
    }

    runtime->executionType(type);
}

void XmlParser::parseInclude(pugi::xml_node node,
                              const std::string &currentFile,
                              LoadConfigFlag flag) {
    pugi::xml_attribute srcAttr = node.attribute("src");

    if(srcAttr) {
        std::string includePath = srcAttr.value();
        if(extra::isAbsolute(includePath)) {
            // if absolute then start from configs dir
            includePath = LMS_CONFIGS + includePath;
        } else {
            // otherwise go from current file's directory
            includePath = extra::dirname(currentFile) + "/" + includePath;
        }
        parseFile(includePath, flag);
    } else {
        errorMissingAttr(node, srcAttr);
    }
}

void XmlParser::parseRuntime(pugi::xml_node node, const std::string &currentFile,
                             LoadConfigFlag flag) {
    pugi::xml_attribute srcAttr = node.attribute("src");
    pugi::xml_attribute nameAttr = node.attribute("name");

    if(srcAttr && nameAttr) {
        std::string includePath = srcAttr.value();
        if(extra::isAbsolute(includePath)) {
            // if absolute then start from configs dir
            includePath = LMS_CONFIGS + includePath;
        } else {
            // otherwise go from current file's directory
            includePath = extra::dirname(currentFile) + "/" + includePath;
        }

        Runtime *temp = m_runtime;
        if(flag == LoadConfigFlag::LOAD_EVERYTHING) {
            m_runtime = new Runtime(nameAttr.value(), m_framework);
            m_framework.registerRuntime(m_runtime);
        } else if (m_framework.hasRuntime(nameAttr.value())) {
            // reload configs && runtime is installed
            m_runtime = m_framework.getRuntimeByName(nameAttr.value());
        } else {
            std::cout << "Runtime is not available: " << nameAttr.value() << std::endl;
        }
        parseFile(includePath, flag);
        m_runtime = temp;
    } else if(! srcAttr) {
        errorMissingAttr(node, srcAttr);
    } else if(! nameAttr) {
        errorMissingAttr(node, nameAttr);
    }
}

void XmlParser::parseModules(pugi::xml_node node,
                             const std::string &currentFile,
                             LoadConfigFlag flag) {
    std::shared_ptr<ModuleWrapper> module = std::make_shared<ModuleWrapper>(m_runtime);

    module->name(node.child("name").child_value());

    pugi::xml_node realNameNode = node.child("realName");

    if(realNameNode) {
        module->libname(Loader<Module>::getModulePath(realNameNode.child_value()));
    } else {
        module->libname(Loader<Module>::getModulePath(module->name()));
    }

    pugi::xml_node executionTypeNode = node.child("executionType");

    module->executionType = ExecutionType::NEVER_MAIN_THREAD;

    if(executionTypeNode) {
        std::string executionType = executionTypeNode.child_value();

        if(! lms::executionTypeByName(executionType, module->executionType)) {
            errorInvalidNodeContent(executionTypeNode, "ONLY_MAIN_THREAD|NEVER_MAIN_THREAD");
        }
    }

    // parse all channel mappings
    for(pugi::xml_node mappingNode : node.children("channelMapping")) {
        pugi::xml_attribute fromAttr = mappingNode.attribute("from");
        pugi::xml_attribute toAttr = mappingNode.attribute("to");
        pugi::xml_attribute priorityAttr = mappingNode.attribute("priority");

        if(! fromAttr) {
            errorMissingAttr(mappingNode, fromAttr);
        }

        if(! toAttr) {
            errorMissingAttr(mappingNode, toAttr);
        }

        if(fromAttr && toAttr) {
            module->channelMapping[fromAttr.value()] = toAttr.value();

            if(priorityAttr) {
                module->channelPriorities[toAttr.value()] = priorityAttr.as_int();
            }
        }
    }

    // parse all config
    for(pugi::xml_node configNode : node.children("config")) {
        pugi::xml_attribute srcAttr = configNode.attribute("src");
        pugi::xml_attribute nameAttr = configNode.attribute("name");
        pugi::xml_attribute userAttr = configNode.attribute("user");

        if(userAttr) {
            std::vector<std::string> allowedUsers =
                    lms::extra::split(std::string(userAttr.value()), ',');

            if(std::find(allowedUsers.begin(), allowedUsers.end(),
                         m_args.argUser) == allowedUsers.end()) {
                continue;
            }
        }

        std::string name = "default";
        if(nameAttr) {
            name = nameAttr.value();
        }

        if(srcAttr) {
            std::string lconfPath = srcAttr.value();

            if(extra::isAbsolute(lconfPath)) {
                lconfPath = LMS_CONFIGS + lconfPath;
            } else {
                lconfPath = extra::dirname(currentFile) + "/" + lconfPath;
            }

            bool loadResult = module->configs[name].loadFromFile(lconfPath);
            if(!loadResult) {
                errorFile(lconfPath);
            } else {
                m_files.push_back(lconfPath);
            }
        } else {
            // if there was no src attribut then parse the tag's content
            parseModuleConfig(configNode, module->configs[name], "");
        }
    }

    if(flag != LoadConfigFlag::ONLY_MODULE_CONFIG) {
        m_runtime->executionManager().installModule(module);
    } else {
        m_runtime->executionManager().bufferModule(module);
    }
}

void XmlParser::parseService(pugi::xml_node node, const std::string &currentFile,
                  LoadConfigFlag flag) {
    std::shared_ptr<ServiceWrapper> service = std::make_shared<ServiceWrapper>(&m_framework);

    service->name(node.child("name").child_value());

    pugi::xml_node realNameNode = node.child("realName");

    if(realNameNode) {
        service->libname(Loader<Service>::getModulePath(realNameNode.child_value()));
    } else {
        service->libname(Loader<Service>::getModulePath(service->name()));
    }

    // parse all config
    for(pugi::xml_node configNode : node.children("config")) {
        pugi::xml_attribute srcAttr = configNode.attribute("src");
        pugi::xml_attribute nameAttr = configNode.attribute("name");
        pugi::xml_attribute userAttr = configNode.attribute("user");

        if(userAttr) {
            std::vector<std::string> allowedUsers =
                    lms::extra::split(std::string(userAttr.value()), ',');

            if(std::find(allowedUsers.begin(), allowedUsers.end(),
                         m_args.argUser) == allowedUsers.end()) {
                continue;
            }
        }

        std::string name = "default";
        if(nameAttr) {
            name = nameAttr.value();
        }

        if(srcAttr) {
            std::string lconfPath = srcAttr.value();

            if(extra::isAbsolute(lconfPath)) {
                lconfPath = LMS_CONFIGS + lconfPath;
            } else {
                lconfPath = extra::dirname(currentFile) + "/" + lconfPath;
            }

            bool loadResult = service->getConfig(name).loadFromFile(lconfPath);
            if(!loadResult) {
                errorFile(lconfPath);
            } else {
                m_files.push_back(lconfPath);
            }
        } else {
            // if there was no src attribut then parse the tag's content
            parseModuleConfig(configNode, service->getConfig(name), "");
        }
    }

    if(flag != LoadConfigFlag::ONLY_MODULE_CONFIG) {
        m_runtime->framework().installService(service);
    } else {
        m_runtime->framework().reloadService(service);
    }
}

void XmlParser::parseFile(const std::string &file, LoadConfigFlag flag) {
    std::ifstream ifs(file);

    if(! ifs.is_open()) {
        errorFile(file);
        return;
    }

    m_files.push_back(file);

    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load(ifs);
    if(! result) {
        errorPugiParseResult(file, result);
        return;
    }

    pugi::xml_node rootNode = doc.child("framework");
    preprocessXML(rootNode, m_args.argFlags);

    for(pugi::xml_node node : rootNode.children()) {
        if(std::string("execution") == node.name()) {
            parseExecution(node, m_runtime);
        } else if(std::string("logging") == node.name()) {
            m_filter.reset(parseLogging(node));
        } else if(std::string("modulesToEnable") == node.name()) {
            parseModulesToEnable(node);
        } else if(std::string("module") == node.name()) {
            parseModules(node, file, flag);
        } else if(std::string("include") == node.name()) {
            parseInclude(node, file, flag);
        } else if(std::string("runtime") == node.name()) {
            parseRuntime(node, file, flag);
        } else if(std::string("service") == node.name()) {
            parseService(node, file, flag);
        } else {
            errorUnknownNode(node);
        }
    }
}

std::vector<std::string> const& XmlParser::files() const {
    return m_files;
}

std::vector<std::string> const& XmlParser::errors() const {
    return m_errors;
}

void XmlParser::errorMissingAttr(pugi::xml_node node, pugi::xml_attribute attr) {
    m_errors.push_back(std::string("Missing attribute ") + attr.name() + " in node " + node.path());
}

void XmlParser::errorInvalidAttr(pugi::xml_node node,
                      pugi::xml_attribute attr,
                      const std::string &expectedValue) {
    m_errors.push_back(std::string("Invalid value for attribute ") + attr.name() +
                     " in node " + node.path() + ", Value is \"" +
                     attr.value() + "\" but expected \"" + expectedValue + "\"");
}

void XmlParser::errorInvalidNodeContent(pugi::xml_node node, const std::string &expected) {
    m_errors.push_back("Invalid text content in node " +  node.path() +
                     ", content: \"" + node.child_value() + "\", expected \"" +
                     expected + "\"");
}

void XmlParser::errorFile(const std::string &file) {
    m_errors.push_back("Could not open file " + file);
}

void XmlParser::errorPugiParseResult(const std::string &file, const pugi::xml_parse_result &result) {
    m_errors.push_back(std::string() + "Failed to parse " + file + " as XML: " +
            std::to_string(result.offset) + " " +  result.description());
}

void XmlParser::errorUnknownNode(pugi::xml_node node) {
    m_errors.push_back("Unknown node " + node.path());
}

std::unique_ptr<logging::ThresholdFilter> XmlParser::filter() {
    return std::move(m_filter);
}

}  // namespace lms

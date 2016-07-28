#include <cstring>
#include <algorithm>
#include <fstream>

#include "lms/internal/xml_parser.h"
#include "lms/internal/os.h"
#include "lms/internal/string.h"

namespace lms {
namespace internal {

template<typename T>
struct PutOnStack {
    std::stack<T> &stack;

    PutOnStack(std::stack<T> &stack, const T &obj) : stack(stack) {
        stack.push(obj);
    }

    ~PutOnStack() {
        stack.pop();
    }
};

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

XmlParser::XmlParser(RuntimeInfo &info) : runtime(info) {}

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

bool XmlParser::parseClock(pugi::xml_node node, ClockInfo &info) {
    std::string clockUnit;
    std::int64_t clockValue = 0;

    pugi::xml_attribute sleepAttr = node.attribute("sleep");
    pugi::xml_attribute compensateAttr = node.attribute("compensate");
    pugi::xml_attribute unitAttr = node.attribute("unit");
    pugi::xml_attribute valueAttr = node.attribute("value");
    pugi::xml_attribute watchDog = node.attribute("watchDog");

    info.slowWarnings = true;

    if (sleepAttr) {
        info.sleep = sleepAttr.as_bool();
    } else {
        // if not enabled attribute is given then the clock is considered
        // to be disabled
        info.sleep = false;
    }

    if (valueAttr) {
        clockValue = valueAttr.as_llong();
    } else {
        info.slowWarnings = false;
        return errorMissingAttr(node, valueAttr);
    }

    if (unitAttr) {
        clockUnit = unitAttr.value();
    } else {
        info.slowWarnings = false;
        return errorMissingAttr(node, unitAttr);
    }

    if (compensateAttr) {
        info.sleepCompensate = compensateAttr.as_bool();
    } else {
        info.sleepCompensate = false;
    }

    if (clockUnit == "hz") {
        info.cycle = Time::fromMicros(1000000 / clockValue);
    } else if (clockUnit == "ms") {
        info.cycle = Time::fromMillis(clockValue);
    } else if (clockUnit == "us") {
        info.cycle = Time::fromMicros(clockValue);
    } else {
        info.slowWarnings = false;
        return errorInvalidAttr(node, unitAttr, "ms/us/hz");
    }

    if (watchDog) {
        info.watchDogEnabled = true;
        if (clockUnit == "hz") {
            info.watchDog = Time::fromMicros(1000000 / watchDog.as_llong());
        } else if (clockUnit == "ms") {
            info.watchDog = Time::fromMillis(watchDog.as_llong());
        } else if (clockUnit == "us") {
            info.watchDog = Time::fromMicros(watchDog.as_llong());
        } else {
            info.watchDogEnabled = false;
            return errorInvalidAttr(node, watchDog, "ms/us/hz");
        }
    }

    return true;
}

bool XmlParser::parseInclude(pugi::xml_node node) {
    pugi::xml_attribute srcAttr = node.attribute("src");

    if(! srcAttr) return errorMissingAttr(node, srcAttr);

    std::string includePath = dirname(m_filestack.top()) + "/" + srcAttr.value();
    parseFile(includePath);
    return true;
}

bool XmlParser::parseModule(pugi::xml_node node, ModuleInfo &info) {
    pugi::xml_attribute nameAttr = node.attribute("name");
    pugi::xml_attribute libAttr = node.attribute("lib");
    pugi::xml_attribute classAttr = node.attribute("class");
    pugi::xml_attribute mainThreadAttr = node.attribute("mainThread");
    pugi::xml_attribute logLevelAttr = node.attribute("log");

    if(! nameAttr) return errorMissingAttr(node, nameAttr);
    if(! libAttr) return errorMissingAttr(node, libAttr);
    if(! classAttr) return errorMissingAttr(node, classAttr);

    info.name = nameAttr.as_string();
    info.lib = libAttr.as_string();
    info.clazz = classAttr.as_string();
    info.mainThread = mainThreadAttr.as_bool();

    logging::Level defaultLevel = logging::Level::ALL;
    if (logLevelAttr) {
        logging::levelFromName(logLevelAttr.as_string(), defaultLevel);
    }
    info.log = defaultLevel;

    // parse all channel mappings
    // TODO This now deprecated in favor for channelHint
    for (pugi::xml_node mappingNode : node.children("channelMapping")) {
        pugi::xml_attribute fromAttr = mappingNode.attribute("from");
        pugi::xml_attribute toAttr = mappingNode.attribute("to");
        pugi::xml_attribute priorityAttr = mappingNode.attribute("priority");

        if (!fromAttr) return errorMissingAttr(mappingNode, fromAttr);
        if (!toAttr) return errorMissingAttr(mappingNode, toAttr);

        int priority = priorityAttr ? priorityAttr.as_int() : 0;
        info.channelMapping[fromAttr.value()] =
            std::make_pair(toAttr.value(), priority);
    }

    for (pugi::xml_node channelNode : node.children("channelHint")) {
        pugi::xml_attribute nameAttr = channelNode.attribute("name");
        pugi::xml_attribute mapToAttr = channelNode.attribute("mapTo");
        pugi::xml_attribute priorityAttr = channelNode.attribute("priority");

        if (!nameAttr) return errorMissingAttr(channelNode, nameAttr);

        std::string mapTo = mapToAttr ? mapToAttr.value() : nameAttr.value();
        int priority = priorityAttr ? priorityAttr.as_int() : 0;

        info.channelMapping[nameAttr.value()] =
            std::make_pair(mapTo, priority);
    }

    // parse all config
    for (pugi::xml_node configNode : node.children("config")) {
        pugi::xml_attribute srcAttr = configNode.attribute("src");
        pugi::xml_attribute nameAttr = configNode.attribute("name");

        std::string name = "default";
        if (nameAttr) {
            name = nameAttr.value();
        }

        if (srcAttr) {
            std::string lconfPath = dirname(m_filestack.top()) + "/" + srcAttr.value();

            bool loadResult = info.configs[name].loadFromFile(lconfPath);
            if (!loadResult) {
                return errorFile(lconfPath);
            } else {
                m_files.push_back(lconfPath);
            }
        } else {
            // if there was no src attribut then parse the tag's content
            parseModuleConfig(configNode, info.configs[name], "");
        }
    }

    return true;
}

bool XmlParser::parseService(pugi::xml_node node, ServiceInfo &info) {
    pugi::xml_attribute nameAttr = node.attribute("name");
    pugi::xml_attribute libAttr = node.attribute("lib");
    pugi::xml_attribute classAttr = node.attribute("class");
    pugi::xml_attribute logLevelAttr = node.attribute("log");

    if(! nameAttr) return errorMissingAttr(node, nameAttr);
    if(! libAttr) return errorMissingAttr(node, libAttr);
    if(! classAttr) return errorMissingAttr(node, classAttr);

    info.name = nameAttr.as_string();
    info.lib = libAttr.as_string();
    info.clazz = classAttr.as_string();

    logging::Level defaultLevel = logging::Level::ALL;
    if (logLevelAttr) {
        logging::levelFromName(logLevelAttr.as_string(), defaultLevel);
    }
    info.log = defaultLevel;

    // parse all config
    for (pugi::xml_node configNode : node.children("config")) {
        pugi::xml_attribute srcAttr = configNode.attribute("src");
        pugi::xml_attribute nameAttr = configNode.attribute("name");

        std::string name = "default";
        if (nameAttr) {
            name = nameAttr.value();
        }

        if (srcAttr) {
            std::string lconfPath = dirname(m_filestack.top()) + "/"  + srcAttr.value();

            bool loadResult = info.configs[name].loadFromFile(lconfPath);
            if (!loadResult) {
                errorFile(lconfPath);
            } else {
                m_files.push_back(lconfPath);
            }
        } else {
            // if there was no src attribut then parse the tag's content
            parseModuleConfig(configNode, info.configs[name], "");
        }
    }

    return true;
}

bool XmlParser::parseFile(std::istream &is, const std::string &file) {
    PutOnStack<std::string> put(m_filestack, file);
    m_files.push_back(file);

    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load(is);
    if (!result) {
        return errorPugiParseResult(file, result);
    }

    pugi::xml_node rootNode = doc.child("lms");
    preprocessXML(rootNode, {});

    for (pugi::xml_node node : rootNode.children()) {
        if (std::string("clock") == node.name()) {
            if(!parseClock(node, runtime.clock)) return false;
        } else if (std::string("module") == node.name()) {
            ModuleInfo module;
            if(parseModule(node, module)) {
                runtime.modules.push_back(module);
            }
        } else if (std::string("include") == node.name()) {
            parseInclude(node);
        } else if (std::string("service") == node.name()) {
            ServiceInfo service;
            if(parseService(node, service)) {
                runtime.services.push_back(service);
            }
        } else {
            errorUnknownNode(node);
        }
    }

    return true;
}

bool XmlParser::parseFile(const std::string &file) {
    std::ifstream ifs(file);

    if (!ifs.is_open()) {
        return errorFile(file);
    }

    return parseFile(ifs, file);
}

std::vector<std::string> const &XmlParser::files() const { return m_files; }

std::vector<std::string> const &XmlParser::errors() const { return m_errors; }

bool XmlParser::errorMissingAttr(pugi::xml_node node,
                                 pugi::xml_attribute attr) {
    m_errors.push_back(std::string("Missing attribute ") + attr.name() +
                       " in node " + node.path());
    return false;
}

bool XmlParser::errorInvalidAttr(pugi::xml_node node, pugi::xml_attribute attr,
                                 const std::string &expectedValue) {
    m_errors.push_back(std::string("Invalid value for attribute ") +
                       attr.name() + " in node " + node.path() +
                       ", Value is \"" + attr.value() + "\" but expected \"" +
                       expectedValue + "\"");
    return false;
}

void XmlParser::errorInvalidNodeContent(pugi::xml_node node,
                                        const std::string &expected) {
    m_errors.push_back("Invalid text content in node " + node.path() +
                       ", content: \"" + node.child_value() +
                       "\", expected \"" + expected + "\"");
}

bool XmlParser::errorFile(const std::string &file) {
    m_errors.push_back("Could not open file " + file);
    return false;
}

bool XmlParser::errorPugiParseResult(const std::string &file,
                                     const pugi::xml_parse_result &result) {
    m_errors.push_back(std::string() + "Failed to parse " + file + " as XML: " +
                       std::to_string(result.offset) + " " +
                       result.description());
    return false;
}

bool XmlParser::errorUnknownNode(pugi::xml_node node) {
    m_errors.push_back("Unknown node " + node.path());
    return false;
}

} // namespace internal
} // namespace lms

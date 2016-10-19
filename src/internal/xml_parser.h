#ifndef LMS_XML_PARSER_H
#define LMS_XML_PARSER_H

#include <vector>
#include <string>
#include <exception>
#include <map>
#include <stack>

#include "pugixml.hpp"
#include "lms/config.h"
#include "lms/logging/level.h"
#include "lms/time.h"

namespace lms {
namespace internal {

struct ModuleInfo {
    std::string name;
    std::string lib;
    std::string clazz;
    bool mainThread = false;
    std::map<std::string, std::pair<std::string, int>> channelMapping;
    std::map<std::string, lms::Config> configs;
    lms::logging::Level log;

    std::pair<std::string, int> mapChannel(const std::string &name) const {
        auto it = channelMapping.find(name);
        if (it != channelMapping.end()) {
            return it->second;
        } else {
            return std::make_pair(name, 0);
        }
    }
};

struct ServiceInfo {
    std::string name;
    std::string lib;
    std::string clazz;
    std::map<std::string, lms::Config> configs;
    lms::logging::Level log;
};

struct ClockInfo {
    lms::Time cycle;
    bool sleep = false;
    bool sleepCompensate = false;
    lms::Time watchDog;
    bool watchDogEnabled = false;
    bool slowWarnings = false;
};

struct RuntimeInfo {
    ClockInfo clock;
    std::vector<ModuleInfo> modules;
    std::vector<ServiceInfo> services;
};

/**
 * @brief Traverse an XML node and evaluate <if> tags with the given condition.
 *
 * The <if> tags get evaluated before an XML documented is parsed.
 *
 * Example:
 * The framework started with --flags=logging,norender
 *
 * <framework>
 * <modulesToEnable>
 *   <if set="logging">
 *     <module>image_logger</module>
 *   </if>
 *   <if notSet="norender">
 *     <module>image_renderer</module>
 *   </if>
 * </modulesToEnable>
 * </framework>
 *
 * This gets preprocessed to:
 *
 * <framework>
 * <modulesToEnable>
 *   <module>image_logger</module>
 * </modulesToEnable>
 * </framework>
 *
 * <if> attributes:
 * - set: a single flag. The flag must be set to evaluate the condition to true.
 * - notSet: a single flag. The flag must not be set to evaluate the condition
 *          to true.
 * - anyOf: comma-separated list of flags. One of the flags must be set to
 *          evaluate the condition to true.
 * - allOf: comma-separated list of flags. All of the flags must be set to
 *          evaluate the condition to true.
 * - nothingOf: comma-separated list of flags. None of the flags must be set
 *          to evaluate the condition to true.
 */
void preprocessXML(pugi::xml_node node, const std::vector<std::string> &flags);

/**
 * @brief Parse <config> tag inside of a <module> tag.
 *
 * The <config> tag gets parsed recursively and the subnode's names get
 * concatenated by '.'.
 *
 * Example:
 * <config>
 *   <device>/dev/ttyACM0</device>
 *   <image>
 *     <width>320</width>
 *     <height>240</height>
 *   </image>
 * </config>
 *
 * This config is equal to
 *
 * device = /dev/ttyACM0
 * image.width = 320
 * image.height = 240
 *
 * @param node The node to parse.
 * @param key Initial config parameter key
 * @param config ModuleConfig object to put the parsed key-value pairs into.
 */
void parseModuleConfig(pugi::xml_node node, Config &config,
                       const std::string &key = "");

class XmlParser {
public:
    XmlParser(RuntimeInfo &info, const std::vector<std::string> &flags = {});

    std::vector<std::string> const &errors() const;
    std::vector<std::string> const &files() const;

    /**
     * @brief Parse the given XML node as <execution>
     * @param rootNode
     * @param clock
     */
    bool parseClock(pugi::xml_node node, ClockInfo &info);

    bool parseInclude(pugi::xml_node node);

    bool parseModule(pugi::xml_node node, ModuleInfo &info);

    bool parseService(pugi::xml_node node, ServiceInfo &info);

    bool parseFile(std::istream &is, const std::string &file);
    bool parseFile(const std::string &file);

private:
    std::vector<std::string> m_errors;
    std::vector<std::string> m_files;
    std::stack<std::string> m_filestack;
    std::vector<std::string> m_flags;

    RuntimeInfo &runtime;

    /**
     * @brief Add parse error: An xml node misses a required attribute.
     * @param node
     * @param attr
     */
    bool errorMissingAttr(pugi::xml_node node, pugi::xml_attribute attr);

    /**
     * @brief Add parse error: An xml node has an invalid attribute value.
     * @param node
     * @param attrName
     * @param attrValue
     * @param expectedValue
     */
    bool errorInvalidAttr(pugi::xml_node node, pugi::xml_attribute attr,
                          const std::string &expectedValue);

    void errorInvalidNodeContent(pugi::xml_node, const std::string &expected);

    bool errorFile(const std::string &file);

    bool errorPugiParseResult(const std::string &file,
                              const pugi::xml_parse_result &result);

    bool errorUnknownNode(pugi::xml_node node);
};

} // namespace internal
} // namespace lms

#endif /* LMS_XML_PARSER_H */

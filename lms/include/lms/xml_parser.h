#ifndef LMS_XML_PARSER_H
#define LMS_XML_PARSER_H

#include <vector>
#include <string>
#include <exception>

#include "pugixml.hpp"
#include "lms/module_config.h"
#include "lms/logger.h"
#include "lms/clock.h"
#include "lms/framework.h"

namespace lms {

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
void parseModuleConfig(pugi::xml_node node,  ModuleConfig &config,
                       const std::string &key = "");

class XmlParser {
public:
    XmlParser(Framework &framework, Runtime* runtime, ArgumentHandler const& args);

    enum class LoadConfigFlag {
        LOAD_EVERYTHING,
        ONLY_MODULE_CONFIG
    };

    std::vector<std::string> const& errors() const;
    std::vector<std::string> const& files() const;

    /**
     * @brief Parse the given XML node as <modulesToEnable>
     * @param node node to parse
     * @param modulesToLoadLists parsed enable modules will be put into this map
     */
    void parseModulesToEnable(pugi::xml_node node);

    /**
     * @brief Parse the given XML node as <logging>
     * @return filter instance
     */
    logging::ThresholdFilter* parseLogging(pugi::xml_node node);

    /**
     * @brief Parse the given XML node as <execution>
     * @param rootNode
     * @param clock
     */
    void parseExecution(pugi::xml_node node, Runtime *runtime);

    void parseInclude(pugi::xml_node node, const std::string &currentFile,
                       LoadConfigFlag flag);

    void parseModules(pugi::xml_node node, const std::string &currentFile,
                      LoadConfigFlag flag);

    void parseService(pugi::xml_node node, const std::string &currentFile,
                      LoadConfigFlag flag);

    void parseFile(const std::string &file, LoadConfigFlag flag);

    void parseRuntime(pugi::xml_node node, const std::string &currentFile,
                                 LoadConfigFlag flag);

    /**
     * @brief parseConfig parses the framework-config
     */
    void parseConfig(XmlParser::LoadConfigFlag flag, const std::string &argLoadConfig);

    std::unique_ptr<logging::ThresholdFilter> filter();
private:
    Framework & m_framework;
    Runtime* m_runtime;
    ArgumentHandler const& m_args;

    std::unique_ptr<logging::ThresholdFilter> m_filter;
    std::vector<std::string> m_errors;
    std::vector<std::string> m_files;

    /**
     * @brief Add parse error: An xml node misses a required attribute.
     * @param node
     * @param attr
     */
    void errorMissingAttr(pugi::xml_node node, pugi::xml_attribute attr);

    /**
     * @brief Add parse error: An xml node has an invalid attribute value.
     * @param node
     * @param attrName
     * @param attrValue
     * @param expectedValue
     */
    void errorInvalidAttr(pugi::xml_node node,
                          pugi::xml_attribute attr,
                          const std::string &expectedValue);

    void errorInvalidNodeContent(pugi::xml_node, const std::string &expected);

    void errorFile(const std::string &file);

    void errorPugiParseResult(const std::string &file,
                              const pugi::xml_parse_result &result);

    void errorUnknownNode(pugi::xml_node node);
};

}  // namespace lms

#endif /* LMS_XML_PARSER_H */

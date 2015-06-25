#ifndef LMS_XML_PARSER_H
#define LMS_XML_PARSER_H

#include <vector>
#include <string>

#include "pugixml.hpp"
#include "lms/type/module_config.h"

namespace lms {

/**
 * @brief Traverse an XML node and evaluate <if> tags with the given condition.
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
void parseModuleConfig(pugi::xml_node node,  type::ModuleConfig &config,
                       const std::string &key = "");

}  // namespace lms

#endif /* LMS_XML_PARSER_H */

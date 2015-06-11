#ifndef LMS_XML_PARSER_H
#define LMS_XML_PARSER_H

#include <vector>
#include <string>

#include "pugixml.hpp"

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

}  // namespace lms

#endif /* LMS_XML_PARSER_H */

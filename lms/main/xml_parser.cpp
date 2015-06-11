#include "lms/xml_parser.h"
#include "lms/extra/string.h"

#include <algorithm>
#include <iostream>

namespace lms {

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

            pugi::xml_attribute anyOfAttr = child.attribute("anyOf");
            pugi::xml_attribute allOfAttr = child.attribute("allOf");
            pugi::xml_attribute nothingOfAttr = child.attribute("nothingOf");

            if(anyOfAttr && !allOfAttr && !nothingOfAttr) {
                result = evaluateAnyOf(lms::extra::split(anyOfAttr.value(), ','), flags);
            } else if(allOfAttr && !nothingOfAttr && !anyOfAttr) {
                result = evaluateAllOf(lms::extra::split(allOfAttr.value(), ','), flags);
            } else if(nothingOfAttr && !anyOfAttr && !allOfAttr) {
                result = evaluateNothingOf(lms::extra::split(nothingOfAttr.value(), ','), flags);
            } else {
                std::cout << "Failed to preprocess XML <if>" << std::endl;
            }

            // if the condition evaluated to true
            if(result) {
                std::cout << "Evaluate to true" << std::endl;
                // then move all children of <if> to be siblings of <if>

                pugi::xml_node moveNode;
                while((moveNode = child.first_child())) {
                    node.insert_move_after(moveNode, child);
                }

                node.remove_child(child);
            } else {
                std::cout << "Evaluate to false" << std::endl;
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

}  // namespace lms

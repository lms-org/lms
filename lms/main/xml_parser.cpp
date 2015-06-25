#include "lms/xml_parser.h"
#include "lms/extra/string.h"

#include <algorithm>
#include <iostream>

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

void parseModuleConfig(pugi::xml_node node, type::ModuleConfig &config,
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

}  // namespace lms

#include "lms/internal/viz.h"

namespace lms {
namespace internal {

void dumpModuleChannelGraph(ModuleChannelGraph<Module *> &graph,
                            DotExporter &dot, const std::string &prefix) {
    std::unordered_set<std::string> modules;

    for (const auto &ch : graph) {
        dot.shape(DotExporter::Shape::BOX);
        dot.label(ch.first); // TODO print typename
        dot.node(prefix + "_" + ch.first);
        dot.reset();
        for (const auto &access : ch.second) {
            int prio = access.priority;
            if (prio != 0) {
                dot.label(std::to_string(prio));
            }
            if (access.permission ==
                ModuleChannelGraph<Module *>::Permission::WRITE) {
                dot.edge(prefix + "_" + access.module->getName(),
                         prefix + "_" + ch.first);
            } else {
                dot.edge(prefix + "_" + ch.first,
                         prefix + "_" + access.module->getName());
            }
            dot.reset();

            modules.insert(access.module->getName());
        }
    }

    for (const auto &mod : modules) {
        dot.label(mod);
        dot.node(prefix + "_" + mod);
        dot.reset();
    }
}

void dumpDAG(const DAG<Module *> &dag, DotExporter &dot,
             const std::string &prefix) {
    for (auto const &pair : dag) {
        dot.label(pair.first->getName());
        dot.node(prefix + "_" + pair.first->getName());
    }

    dot.reset();

    for (auto const &pair : dag) {
        std::string from = pair.first->getName();

        for (auto const &to : pair.second) {
            dot.edge(prefix + "_" + to->getName(), prefix + "_" + from);
        }
    }
}

} // namespace internal
} // namespace lms

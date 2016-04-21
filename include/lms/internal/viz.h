#ifndef LMS_INTERNAL_VIZ_H
#define LMS_INTERNAL_VIZ_H

#include <string>
#include "dot_exporter.h"
#include "module_channel_graph.h"
#include "dag.h"
#include "../module.h"

namespace lms {
namespace internal {

void dumpModuleChannelGraph(ModuleChannelGraph<Module*>& graph, DotExporter &dot, const std::string &prefix);

void dumpDAG(const DAG<Module*> &dag, DotExporter &dot, const std::string &prefix);

}  // namespace internal
}  // namespace lms

#endif // LMS_INTERNAL_VIZ_H

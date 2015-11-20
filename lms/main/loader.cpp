#include <lms/loader.h>
#include <lms/logger.h>
#include <lms/module.h>
#include <lms/extra/os.h>
#include <lms/extra/string.h>

namespace lms {

Loader::Loader() : logger("lms.Loader") {
}

void Loader::addModulePath(std::string const& path, int recursion) {
    std::vector<std::string> list;
    lms::extra::listDir(path, list);

    for(std::string const& child : list) {
        std::string childPath = path + "/" + child;
        lms::extra::FileType type = lms::extra::fileType(childPath);

        if(type == lms::extra::FileType::REGULAR_FILE &&
                lms::extra::startsWith(child, "lib") &&
                lms::extra::endsWith(child, ".so")) {
            m_pathMapping[child] = childPath;
        } else if(type == lms::extra::FileType::DIRECTORY && recursion > 0) {
            addModulePath(childPath, recursion - 1);
        }
    }
}

}  // namespace lms

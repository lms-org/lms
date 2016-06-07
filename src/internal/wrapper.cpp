#include "lms/internal/wrapper.h"

namespace lms {
namespace internal {

Wrapper::~Wrapper() {}

std::string Wrapper::lib() const { return m_lib; }

void Wrapper::lib(std::string const &lib) { m_lib = lib; }

std::string Wrapper::name() const { return m_name; }

void Wrapper::name(std::string const &name) { m_name = name; }

std::string Wrapper::clazz() const { return m_class; }

void Wrapper::clazz(std::string const &clazz) { m_class = clazz; }

} // namespace internal
} // namespace lms

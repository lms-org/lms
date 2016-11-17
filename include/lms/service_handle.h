#ifndef LMS_SERVICE_HANDLE_H
#define LMS_SERVICE_HANDLE_H

#include <memory>
#include <mutex>

#include "service.h"

namespace lms {

template <class T> class ServiceHandle {
public:
    ServiceHandle(std::shared_ptr<Service> service)
        : m_service(service), m_lock(service->getMutex()), m_valid(true) {}

    ServiceHandle() : m_valid(false) {}

    T *operator->() { return static_cast<T *>(m_service.get()); }

    operator bool() const { return m_valid; }

    bool isValid() const { return m_valid; }

private:
    std::shared_ptr<Service> m_service;
    std::unique_lock<std::mutex> m_lock;
    bool m_valid;
};

} // namespace lms

#endif // LMS_SERVICE_HANDLE_H

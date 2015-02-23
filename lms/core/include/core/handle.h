#ifndef CORE_HANDLE_H
#define CORE_HANDLE_H

#include <memory.h>
class DataManager;

class GenericHandle {
private: 
    GenericHandle(const GenericHandle &);
protected: 
    GenericHandle( DataManager *dm ) { _datamanager = dm; }
public:
    virtual void set_default(void *data) = 0;
    virtual void destroy(void *data) = 0;
    
    virtual void* get_raw() = 0;
    
    void set_raw(void *data, size_t size) {
        memcpy(get_raw(), data, size);
    }

    virtual int getHandle() = 0;
    int getSize();
protected: 
    void get(int handle, void**data);

private:
    DataManager* datamanager() { return _datamanager; }
    DataManager *_datamanager;
};

template <typename _T>
class Handle : public GenericHandle {
friend class DataManager; 
private: 
    Handle (DataManager *dm, int h) : GenericHandle(dm), handle(h) { }
    Handle( const Handle &h);
    int handle;
    
    // durch Destructor ersetzen
    void destroy(void* data) { ((_T*)data) -> ~_T(); }
public: 
    void set_default(void* data) {
        *((_T*)data) = _T();
    }
    
    void* get_raw() { return get(); }

    _T* get() {
        _T* t; 
        GenericHandle::get(handle, (void**)&t);
        return t;
    }
    
    _T& ref() {
        return *get();
    }
    
    void set(const _T& t) {
        ref() = t;
    }
    
    int getHandle() {
        return handle;
    }
};
#endif // CORE_HANDLE_H

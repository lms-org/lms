#ifndef LMS_DATA_CHANNEL
#define LMS_DATA_CHANNEL
#include <cstring>
namespace lms {
/*Thoughts:

class DataChannel<T>{
T* is a shared-pointer

T* main;//is created by the framework if buffered() returns false

//buffer für jede runtime
vector<T*>* buffer;

vector<DataChannel<T>> getBuffer(){
//Übergibt alle verfügbaren T*
}



//called after each cycle of the runtime for each dataChannel

bufferCycle(){
    if(buffered()){
        //clears the buffer
    }else if(sharesData()){
        //kopiert T* zu der anderen Runtime
        // Man buffert den DatenKanal in der runtime und fügt ihn beim nächsten Zyklus in den buffer des DataChannels ein

        //muss den runtimeBuffer mutexen
    }
}

operator -> {
if(buffered()){
    return last element of the buffer
}else{
    return T*
}
}


//################Utils-methods
int getCycle()
bool hasReader()
bool hasWriter()
bool sharesData(){
returns true if the data is accessed by another runtime
}
bool buffered(){
receives data from another runtime (hasWriters returns false!)
}

//################Utils-methods-END



};//end class

cycle(){

...

if(dataChannel.sharesData()){
    dataChannel.bufferData()
}

}
*/

template<typename T> class DataChannel{


    class PointerWrapper {
    public:
        virtual ~PointerWrapper() {}
        virtual void* get() = 0;
    };

    template<typename T>
    class PointerWrapperImpl : public PointerWrapper {
    public:
        PointerWrapperImpl() {}
        PointerWrapperImpl(const T &data) : data(data) {}
        void* get() { return &data; }
        void set(const T &data) { this->data = data; }
        T data;
    };

    struct DataChannelInformation {
        DataChannelInformation() : dataWrapper(nullptr), dataSize(0), exclusiveWrite(false) {}

        PointerWrapper *dataWrapper; // TODO hier auch unique_ptr möglich
        size_t dataSize; // currently only for idiot checks
        std::string dataTypeName;
        size_t dataHashCode;
        bool serializable;
        bool exclusiveWrite;
        //ModuleList readers;
        //ModuleList writers;
    } info;

    /**
     * @brief TODO
     * @param name data channel name
     * @return -1 if the type is invalid, 0 if the type of the DataChannel is subtype of the given one, 1 if the given type is the same or subtype of the DataChannel-type
     */
    template<typename L>
    int checkType(const std::string &name) {
        // check for hash code of data types
        if(info.dataHashCode != typeid(L).hash_code()) {
            logger.error() << "Requested wrong data type for channel " << name << std::endl
                << "Channel type is " << channel.dataTypeName << ", requested was " << extra::typeName<T>();
            return -1;
        }

        // check for size of data types
        // TODO this is not longer necessary
        if(info.dataSize != sizeof(T)) {
            logger.error() << "Wrong data size for channel " << name << "!" << std::endl
                << "Requested " << sizeof(T) << " but is " << channel.dataSize;
            return -1;
        }

        return 0;
    }

};
}

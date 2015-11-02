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
#include <vector>
#include <cstring>
#include <memory>

class DataChannelBase{
public:
    virtual ~DataChannelBase;

    struct DataChannelInformation {
        DataChannelInformation() : dataSize(0), exclusiveWrite(false) {}
        size_t dataSize; // currently only for idiot checks
        std::string dataTypeName;
        size_t dataHashCode;
        bool serializable;
        bool exclusiveWrite;
    };

protected:
    std::shared_ptr<DataChannelInformation> info;
    int m_cycle; //TODO not sure where/how to set it // -> (not const) sets current cycle-count + returns const ->
    std::vector<std::string> readers;
    std::vector<std::string> writers;


public:
    /**
     * @brief getCycle
     * @return cycle in which the object was created
     */
    int getCycle(){
        return m_cycle;
    }

    bool hasReader(){
        return readers.size() > 0;
    }
    /**
     * @brief hasWriter
     * @return
     */
    bool hasWriter(){
        return writers.size() > 0;
    }

    /**
     * @brief sharesData
     * @return true if the data is accessed by another runtime
     */
    bool sharesData(){
        return false; //TODO
    }

    /**
     * @brief buffered
     * @return true if it receives data from another runtime (hasWriters returns false!)
     */
    bool buffered(){
        return false; //TODO
    }
};

template<typename T> class DataChannel: public DataChannelBase{
private:
    std::shared_ptr<T*> main;
    std::shared_ptr<std::vector<DataChannel<T>>> m_buffer;

public:

    std::vector<DataChannel<T>> &buffer(){
    //Übergibt alle verfügbaren T*
        return m_buffer.get();
    }



    //called after each cycle of the runtime for each dataChannel

    void bufferCycle(){
        if(buffered()){
            //clears the buffer
        }else if(sharesData()){
            //kopiert T* zu der anderen Runtime
            // Man buffert den DatenKanal in der runtime und fügt ihn beim nächsten Zyklus in den buffer des DataChannels ein

            //muss den runtimeBuffer mutexen
        }
    }

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


template<typename T> class ReadDataChannel: public DataChannel<T>{

    T* operator ->(){
        if(buffered()){
            if(buffer.size() > 0)
                return buffer[buffer.size() -1];
            else
                return nullptr;
        }else{
            return main;
        }
    }

};
template<typename T> class WriteDataChannel: public DataChannel<T>{

    T* operator ->(){
        //TODO set the cycle-count
        return main;
    }
};


}//namespace


#include "protobuf_socket.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <memory>

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

namespace lms {
namespace internal {

ProtobufSocket::ProtobufSocket(int fd) : fd(fd) {}

bool ProtobufSocket::writeMessage(const google::protobuf::Message &message) {
    int siz = message.ByteSize() + 4;
    std::unique_ptr<char[]> pkt(new char [siz]);
    google::protobuf::io::ArrayOutputStream aos(pkt.get(),siz);
    google::protobuf::io::CodedOutputStream coded_output(&aos);
    coded_output.WriteLittleEndian32(message.ByteSize());
    message.SerializeToCodedStream(&coded_output);

    return ::send(fd, pkt.get(),siz,0) != -1;
}

bool ProtobufSocket::readMessage(google::protobuf::Message &message) {
    char buffer[4];
    int bytesRead = recv(fd, buffer, 4, MSG_PEEK);
    if(bytesRead == -1) {
        // error
        return false;
    } else if(bytesRead == 0) {
        // closed
        return false;
    }

    // parse message length
    google::protobuf::uint32 messageSize;
    {
    google::protobuf::io::ArrayInputStream ais(buffer, 4);
    google::protobuf::io::CodedInputStream coded_input(&ais);
    coded_input.ReadLittleEndian32(&messageSize);
    }

    // parse message
    {
    std::unique_ptr<char[]> messageBuffer(new char[4 + messageSize]);
    bytesRead = recv(fd, messageBuffer.get(), 4+messageSize, MSG_WAITALL);
    if(bytesRead == -1) {
        // error
        return false;
    } else if(bytesRead == 0) {
        return false;
    }
    google::protobuf::io::ArrayInputStream ais(messageBuffer.get(),messageSize+4);
    google::protobuf::io::CodedInputStream coded_input(&ais);
    std::uint32_t messageSize2;
    coded_input.ReadLittleEndian32(&messageSize2);
    google::protobuf::io::CodedInputStream::Limit msgLimit = coded_input.PushLimit(messageSize);
    message.ParseFromCodedStream(&coded_input);
    coded_input.PopLimit(msgLimit);
    }

    return true;
}

int ProtobufSocket::getFD() const {
    return fd;
}

void ProtobufSocket::close() {
    ::close(fd);
}

}  // namespace internal
}  // namespace lms

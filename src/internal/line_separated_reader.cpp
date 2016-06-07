#include <unistd.h>
#include <algorithm>
#include <iostream>

#include "lms/internal/line_separated_reader.h"

namespace lms {
namespace internal {

LineReader::LineReader(int fd) : fd(fd),
    buffer(4096), // resize buffer to 4 KB
    bufferUsed(0) // nothing is used initially
{}

bool LineReader::readLine(std::string& line) {
    if(readLineFromBuffer(line)) {
        return true;
    }

    int retval = ::read(fd, buffer.data() + bufferUsed, buffer.size() - bufferUsed);
    if(retval > 0) {
        bufferUsed += retval;

        if(readLineFromBuffer(line)) {
            return true;
        }
    }

    return false;
}

bool LineReader::readLineFromBuffer(std::string &line) {
    auto end = buffer.begin() + bufferUsed;
    auto found = std::find(buffer.begin(), end, '\n');
    if(found != end) {
        line = std::string(buffer.begin(), found);
        std::copy(found + 1, buffer.begin() + bufferUsed, buffer.begin());
        bufferUsed -= found - buffer.begin() + 1;
        return true;
    }

    return false;
}

LineWriter::LineWriter(int fd) : fd(fd) {}

void LineWriter::writeLine(const std::string& line) {
    ::write(fd, line.c_str(), line.length());
    char sep = '\n';
    ::write(fd, &sep, 1);
}

void LineWriter::writeLine() {
    char sep = '\n';
    ::write(fd, &sep, 1);
}

}  // namespace internal
}  // namespace lms

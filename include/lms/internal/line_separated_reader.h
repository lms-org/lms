#pragma once

#include <string>
#include <vector>

namespace lms {
namespace internal {

class LineReader {
public:
    LineReader(int fd);
    bool readLine(std::string &line);

private:
    int fd;
    std::vector<char> buffer;
    size_t bufferUsed;

    /**
     * @brief Try to read a line from the internal buffer.
     * @param line line will be placed in this reference
     * @return true, if a line was read, false otherwise
     */
    bool readLineFromBuffer(std::string &line);
};

class LineWriter {
public:
    LineWriter(int fd);
    void writeLine(const std::string &line);
    void writeLine();

private:
    int fd;
};
}
}

#ifndef FOREST_BUFFER_H
#define FOREST_BUFFER_H

#include <string>
#include"BufferUtil.h"
namespace Forest
{
    class Buffer
    {
    public:
        Buffer();
        ~Buffer();

        size_t readDataFromFd(const int fd);
        size_t writeDataToFd(const int fd);
    private:
        void expandBuffer(size_t size, int n);
    private:
        size_t _totalLen;
        size_t _chainSize;
        bufferChain* _first;
        bufferChain* _last;
        bufferChain* _lastWithData;
    };
}

#endif
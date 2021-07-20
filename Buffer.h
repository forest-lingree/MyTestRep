#ifndef FOREST_BUFFER_H
#define FOREST_BUFFER_H

#include <string>
#include <memory>
#include <sys/ioctl.h>

#include"BufferUtil.h"

namespace Forest
{
    //todo: use shared_ptr to replace bufferChain*
    //      so that we can free chains automatically
    class Buffer
    {
    public:
        Buffer();
        ~Buffer();

        size_t readDataFromFd(const int fd);
        size_t writeDataToFd(const int fd);
    private:
        bool addDataToBuffer(const char *dataIn, size_t datalen);
        int expandBuffer(size_t size/*, int n */);
        void removeDataAndReuseChain(size_t n);
        void insertBufferChain(bufferChain* chain); //insert bufferChain after lastWithData;
        bufferChain *creatBufferChain(size_t size);
        std::shared_ptr<struct iovec> generateIovec(const int n, bufferChain *&pChain);
        void destroyBuffer();
    private:
        size_t _totalLen;
        size_t _chainSize;
        bufferChain* _first;
        bufferChain* _last;
        bufferChain* _lastWithData;
    };
}

#endif
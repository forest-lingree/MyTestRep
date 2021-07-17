#ifndef FOREST_BUFFER_H
#define FOREST_BUFFER_H

#include <string>
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
        void expandBuffer(size_t size/*, int n */);
        void insertBufferChain(bufferChain* chain); //insert bufferChain after lastWithData;
        bufferChain *creatBufferChain(size_t size);
    private:
        size_t _totalLen;
        size_t _chainSize;
        bufferChain* _first;
        bufferChain* _last;
        bufferChain* _lastWithData;
    };
}

#endif
#include<iostream>
#include <sys/ioctl.h>
#include"Buffer.h"

namespace Forest{
    Buffer::Buffer()
    :_first(nullptr),
    _last(nullptr),
    _totalLen(0),
    _chainSize(0)
    {
        _lastWithData = _first;
    }

    size_t Buffer::readDataFromFd(const int fd)
    {
        bufferChain* pChain= nullptr;
        int n = 0,nvecs = 0,i = 0;
        size_t howmuch = 0;
        if(ioctl(fd, FIONREAD,&howmuch) < 0)
        {
            exit(0);
        }

        
    }

    void Buffer::expandBuffer(size_t size, int n)
    {
        
    }
}
#include<iostream>
#include <assert.h>
#include <sys/ioctl.h>
#include <sys/uio.h>
#include"Buffer.h"

using namespace std;
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
            std::cout << "ioctrl error";
            throw(std::length_error("data size mast be positive"));
        }
        int chainCnt = 0;
        try
        {
            chainCnt = expandBuffer(howmuch);
        }
        catch(...)
        {
            std::cout << "expand buffer failed /n";
            throw;
        }
        shared_ptr<struct iovec> vecs = generateIovec(chainCnt, pChain);
        n = readv(fd, vecs.get(), chainCnt);
        if(n == -1 || n == 0)
        {
            return n;
        }
        for(i = 0; i < chainCnt; ++i)
        {
            pChain->_off += vecs.get()[i].iov_len;
            pChain = pChain->_next;
        }
        pChain = _first;
        while(pChain->_next != nullptr && pChain->_next->_off != 0)
        {
            pChain = pChain->_next;
        }
        _lastWithData = pChain;
        _totalLen += n;
        return n;
    }

    int Buffer::expandBuffer(size_t size /*, int n */) 
    {
        bufferChain *chain = nullptr, *tmp = nullptr;
        size_t avail = 0;
        int used = 0;

        //int n can be used to control the loop times;
        //incase size_t size is very large, 
        //but _lastWithData is followed by lots of small buffers
        //(can be added later)
        for(chain = _lastWithData; chain != nullptr; chain = chain->_next)
        {
            if(chain->_off > 0)
            {
                size_t space = chain->_bufferLen - chain->_misalign - chain->_off;
                if(space > 0)
                {
                    avail += space;
                    ++used;
                }
            }
            else
            {
                chain->_misalign = 0;
                avail += chain->_bufferLen;
                ++used;
            }
            if(avail >= size)
            {
                return used;
            }

            size -= avail;
            try
            {
                tmp = creatBufferChain(size);
            }
            catch(...)
            {
                std::cout << "creatBufferChain failed";
            }
            insertBufferChain(tmp);
            ++used;
            return used;
        }

    }

    void Buffer::insertBufferChain(bufferChain *chain)
    {
        if(_lastWithData == nullptr)
        {
            assert(_first == nullptr);
            _first = _last = chain;
            _lastWithData = chain;
        }
        else
        {
            bufferChain *tmp = _lastWithData->_next;
            _lastWithData->_next = chain;
            chain->_next = tmp;
        }
        _totalLen += chain->_off;
        ++_chainSize;
    }

    bufferChain* Buffer::creatBufferChain(size_t size)
    {
        size_t alloc = 0;
        bufferChain *chain = nullptr;
        void* mmPtr = nullptr;

        if(size > kBufferChainMax - kBufferChainSize)
        {
            std::cout << "SIZE IS TOO LARGE";
            throw(std::length_error("size is too large"));
        }
        size += kBufferChainSize;
        if(size < kBufferChainMax / 2)
        {
            alloc = kBufferSizeMin;
            while(alloc < size)
            {
                alloc << 1;
            }
        }
        else
        {
            alloc = size;
        }
        if((mmPtr = ::operator new(alloc)) == nullptr)
        {
            std::cout << "memory alloc failed";
            throw(std::logic_error("memory alloc failed"));
        }
        chain = ::new(mmPtr)bufferChain(); //placement new;
        chain->_bufferLen = alloc - kBufferChainSize;
        chain->_buffer = reinterpret_cast<char*>(chain + 1);
        return chain;
    }

    std::shared_ptr<struct iovec> Buffer::generateIovec(const int n, bufferChain *&pChain)
    {
        std::shared_ptr<struct iovec> iovecs(new struct iovec[n]);
        bufferChain *chain = nullptr;
        bufferChain *firstAvailable = nullptr;
        size_t all_avail = 0;
        int i = 0;
        firstAvailable = _lastWithData;
        if(firstAvailable->_bufferLen - firstAvailable->_misalign - firstAvailable->_off == 0)
        {
            firstAvailable = firstAvailable->_next;
        }
        assert(firstAvailable != nullptr);

        for(;i < n; ++i)
        {
            size_t avail = chain->_bufferLen - chain->_misalign - chain->_off;
            iovecs.get()[i].iov_base = (void*)(chain->_buffer + chain->_misalign + chain->_off);
            iovecs.get()[i].iov_len = avail;
        }
        pChain = firstAvailable;
        return iovecs;
    }
}


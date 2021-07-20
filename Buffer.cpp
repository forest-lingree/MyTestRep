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

    Buffer::~Buffer()
    {
        //destroyBuffer();
        
        bufferChain* chain = _first;
        bufferChain* next = nullptr;
        while(chain != nullptr)
        {
            next=chain->_next;
            ::delete chain;
            chain = next;
        }
    }
    bool Buffer::addDataToBuffer(const char *dataIn, size_t datalen)
    {
        bufferChain *chain = nullptr, *tmp = nullptr;
    }

    void Buffer::destroyBuffer()
    {
        bufferChain* chain = _first;
        bufferChain* next = nullptr;
        while(chain != nullptr)
        {
            next=chain->_next;
            ::delete chain;
            chain = next;
        }
    }

    size_t Buffer::readDataFromFd(const int fd)
    {
        bufferChain* pChain= nullptr;
        int n = 0,nvecs = 0,i = 0;
        size_t howmuch = 0;
        //this is a temporary slotion
        //here is the case:
        //after connected,before data sent
        //ioctl is non-blocking
        //howmuch = 0
        //process is blocked at the fllowing readv
        while(howmuch == 0)
        {        
            if(ioctl(fd, FIONREAD,&howmuch) < 0)
            {
                std::cout << "ioctrl error";
                throw(std::length_error("data size mast be positive"));
            }
            if(howmuch == 0)
            {
                std::cout <<"fd to read is empty, looping";
            }
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

    size_t Buffer::writeDataToFd(const int fd)
    {
        size_t howmuch = _totalLen;
        int i=0;
        size_t n=0;
        bufferChain* chain = _first;
        struct iovec vec[_chainSize];
        while(howmuch>0)
        {
            vec[i].iov_base=(void*)(chain->_buffer+chain->_misalign);
            if(howmuch > chain->_off)
            {
                vec[i].iov_len=chain->_off;
                howmuch-=chain->_off;
            }
            else {
                vec[i].iov_len = howmuch;
                howmuch = 0;
            }
            i++;
            chain = chain->_next;
        }
        n = ::writev(fd,vec,i);
        if(n<0)
        {
            std::cout << "writev error,"<< "error number is" <<errno;
            if(errno == EWOULDBLOCK || errno == EINTR)
            {
                return errno;
            }
        }
        removeDataAndReuseChain(n);
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

    void Buffer::removeDataAndReuseChain(size_t len)
    {
        bufferChain* chain,*next,*first;
        size_t remaining,old_len;
        old_len=_totalLen;
        if(old_len == 0)
            return;
		assert(_first);
        if(len >= old_len)
        {
        	chain =_first;
        	while(chain != _lastWithData->_next)
			{
        		chain->_misalign=0;
        		chain->_off = 0;
        		chain = chain->_next;
			}
            _lastWithData=_first;
            _totalLen=0;
        }
        else
        {
            _totalLen-=len;
            remaining=len;
            first = _first;
            for(chain = first;remaining>=chain->_off;chain=next)
            {
                next=chain->_next;
                remaining-=chain->_off;
                chain->_misalign=0;
                chain->_off=0;
                if(next != _lastWithData->_next)
				{
					_first = next;
					_last->_next=chain;
					_last=chain;
				}
            }
            assert(remaining < chain->_off);
            chain->_misalign+=remaining;
            chain->_off -= remaining;
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
        chain = firstAvailable;
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

